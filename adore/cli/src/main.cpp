#include <iostream>
#include <condition_variable>
#include "raylib.h"
#include "lua.h"
#include "lualib.h"
#include "lute/uvstate.h"
#include "lute/runtime.h"
#include "lute/climain.h"
#include "lute/require.h"
#include "Luau/Common.h"
#include "Luau/Require.h"
#include "Luau/FileUtils.h"
#include "Luau/Compiler.h"
#include "adore/window.h"
#include "adore/graphics.h"
#include "adore/color.h"
#include "adore/gui.h"
#ifdef ADORE_BLACKMAGIC
#include "adore/blackmagic.h"
#include "adore/hyperdeck.h"
#include "adore/timecode.h"
#endif

namespace adore {


Luau::CompileOptions copts()
{
    Luau::CompileOptions result = {};
    result.optimizationLevel = 2;
    result.debugLevel = 2;
    result.typeInfoLevel = 1;
    result.coverageLevel = 0;

    return result;
}

static bool setupArguments(lua_State* L, int argc, char** argv)
{
    if (!lua_checkstack(L, argc))
        return false;

    for (int i = 0; i < argc; ++i)
        lua_pushstring(L, argv[i]);

    return true;
}

static bool runBytecode(Runtime& runtime, const std::string& bytecode, const std::string& chunkname, lua_State* GL, int program_argc, char** program_argv)
{
    // module needs to run in a new thread, isolated from the rest
    lua_State* L = lua_newthread(GL);

    // new thread needs to have the globals sandboxed
    luaL_sandboxthread(L);

    if (luau_load(L, chunkname.c_str(), bytecode.data(), bytecode.size(), 0) != 0)
    {
        if (const char* str = lua_tostring(L, -1))
            fprintf(stderr, "%s", str);
        else
            fprintf(stderr, "Failed to load bytecode");

        lua_pop(GL, 1);
        return false;
    }
    
    if (!setupArguments(L, program_argc, program_argv))
    {
        fprintf(stderr, "Failed to pass arguments to Luau");
        lua_pop(GL, 1);
        return false;
    }

    runtime.GL = GL;
    runtime.runningThreads.push_back({true, getRefForThread(L), program_argc});

    lua_pop(GL, 1);

    return true;
}

static bool runFile(Runtime& runtime, const char* name, lua_State* GL, int program_argc, char** program_argv)
{
    if (isDirectory(name))
    {
        fprintf(stderr, "Error: %s is a directory\n", name);
        return false;
    }

    std::optional<std::string> source = readFile(name);
    if (!source)
    {
        fprintf(stderr, "Error opening %s\n", name);
        return false;
    }

    std::string chunkname = "@" + normalizePath(name);

    std::string bytecode = Luau::compile(*source, copts());
    
    adore::runBytecode(runtime, bytecode, chunkname, GL, program_argc, program_argv);
    bool quit = false;
    bool result = true;
    bool windowCreated = false;

    while (!quit) {
        windowCreated = windowCreated || IsWindowReady();
        if (windowCreated) {
            if (WindowShouldClose()) {
                break;
            }

            runtime.schedule([&runtime]() {
                lua_State* L = runtime.globalState.get();

                // remember and reserve stack space so we don't violate call frame limits
                int base = lua_gettop(L);
                lua_checkstack(L, 8);

                lua_getglobal(L, "_WINDOW");
                if (lua_istable(L, -1)) {
                    lua_getfield(L, -1, "update");
                    if (lua_isfunction(L, -1)) {
                        // push delta time
                        float dt = GetFrameTime();
                        lua_pushnumber(L, dt);
                        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
                            runtime.reportError(L);
                            lua_pop(L, 1);
                        }
                    } else {
                        lua_pop(L, 1);
                    }

                    lua_getfield(L, -1, "draw");
                    if (lua_isfunction(L, -1)) {
                        BeginDrawing();
                        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
                            runtime.reportError(L);
                            lua_pop(L, 1);
                        }
                        EndDrawing();
                    } else {
                        lua_pop(L, 1);
                    }
                }

                // restore stack to its original state to avoid leaving extra items on the stack
                lua_settop(L, base);
            });
        } else if (!runtime.hasWork()) {
            quit = true;
            continue;
        }

        if (runtime.hasWork()) {
            auto step = runtime.runOnce();

            if (auto err = Luau::get_if<StepErr>(&step))
            {
                if (err->L == nullptr)
                {
                    fprintf(stderr, "lua_State* L is nullptr");
                    return false;
                }

                runtime.reportError(err->L);

                // ensure we exit the process with error code properly
                if (!runtime.hasWork()) {
                    quit = true;
                    result = false;
                    continue;
                }
            }

            if (!windowCreated) {
                // yield to avoid busy loop
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }

    if (windowCreated) {
        CloseWindow();
    }

    return result;
}

static std::optional<std::string> getWithRequireByStringSemantics(std::string filePath)
{
    std::string normalized = normalizePath(std::move(filePath));

    std::string rootOfPath;
    std::string restOfPath = normalized;
    if (size_t firstSlash = normalized.find_first_of("\\/"); firstSlash != std::string::npos)
    {
        rootOfPath = normalized.substr(0, firstSlash);
        restOfPath = normalized.substr(firstSlash + 1);
    }

    std::optional<ModulePath> mp = ModulePath::create(std::move(rootOfPath), std::move(restOfPath), isFile, isDirectory);
    if (!mp)
        return std::nullopt;

    ResolvedRealPath resolved = mp->getRealPath();
    if (resolved.status != NavigationStatus::Success)
        return std::nullopt;

    if (resolved.type == ResolvedRealPath::PathType::File)
        return resolved.realPath;

    return std::nullopt;
};

static std::optional<std::string> getValidPath(std::string filePath)
{
    if (std::optional<std::string> path = getWithRequireByStringSemantics(filePath))
        return *path;

    // Only fallback to checking .lute/* if the original path has no extension.
    if (filePath.find('.') != std::string::npos)
        return std::nullopt;

    std::string fallbackPath = joinPaths(".lute", filePath);
    size_t fallbackSize = fallbackPath.size();

    for (const auto& ext : {".luau", ".lua"})
    {
        fallbackPath.resize(fallbackSize);
        fallbackPath += ext;

        if (isFile(fallbackPath))
            return fallbackPath;
    }

    return std::nullopt;
}

void displayRunHelp()
{
	printf("Usage: adore [options] <file>\n");
	printf("\n");
	printf("Options:\n");
	printf("  -h, --help          Display this help message\n");
	printf("\n");
}

void setupLuaState(lua_State* L) {
	// Open our own libraries here
	std::vector<std::pair<const char*, lua_CFunction>> libs = {{
        {"@adore/window", adoreopen_window},
        {"@adore/graphics", adoreopen_graphics},
        {"@adore/color", adoreopen_color},
        {"@adore/gui", adoreopen_gui},
#ifdef ADORE_BLACKMAGIC
        {"@adore/blackmagic", adoreopen_blackmagic},
        {"@adore/hyperdeck", adoreopen_hyperdeck},
        {"@adore/timecode", adoreopen_timecode},
#endif
    }};

    for (const auto& [name, func] : libs)
    {
        lua_pushcfunction(L, luarequire_registermodule, nullptr);
        lua_pushstring(L, name);
        func(L);
        lua_call(L, 2, 0);
    }
}

int handleRunCommand(int argc, char** argv, int argOffset)
{
    std::string filePath;
    int program_argc = 0;
    char** program_argv = nullptr;

    for (int i = argOffset; i < argc; ++i)
    {
        const char* currentArg = argv[i];

        if (strcmp(currentArg, "-h") == 0 || strcmp(currentArg, "--help") == 0)
        {
            displayRunHelp();
            return 0;
        }
        else if (currentArg[0] == '-')
        {
            fprintf(stderr, "Error: Unrecognized option '%s'\n\n", currentArg);
            displayRunHelp();
            return 1;
        }
        else
        {
            filePath = currentArg;
            program_argc = argc - i;
            program_argv = &argv[i];
            break;
        }
    }

    if (filePath.empty())
    {
        fprintf(stderr, "Error: No file specified.\n\n");
        displayRunHelp();
        return 1;
    }

    Runtime runtime;
    lua_State* L = setupCliState(runtime, setupLuaState);

    lua_getglobal(L, "_WINDOW");
    lua_setreadonly(L, -1, false);
    lua_pop(L, 1);


    std::optional<std::string> validPath = getValidPath(filePath);
    if (!validPath)
    {
        std::cerr << "Error: File '" << filePath << "' does not exist.\n";
        return 1;
    }

    bool success = runFile(runtime, validPath->c_str(), L, program_argc, program_argv);
    return success ? 0 : 1;
}

static int assertionHandler(const char* expr, const char* file, int line, const char* function)
{
    printf("%s(%d): ASSERTION FAILED: %s\n", file, line, expr);
    return 1;
}

}



int main(int argc, char** argv) {
    UvGlobalState uvState(argc, argv);
    Luau::assertHandler() = adore::assertionHandler;

	return adore::handleRunCommand(argc, argv, 1);
}
