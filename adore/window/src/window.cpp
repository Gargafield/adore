#include "adore/window.h"
#include <memory>
#include "raylib.h"
#include <iostream>

namespace window {

static bool initialized = false;

bool is_window_initialized() {
    return initialized;
}

int init(lua_State* L) {
    if (initialized) { \
        luaL_errorL(L, "Window already initialized"); \
    }
    initialized = true;

    int width = luaL_checkinteger(L, 1);
    int height = luaL_checkinteger(L, 2);
    const char* title = luaL_checkstring(L, 3);
    InitWindow(width, height, title);
    return 0;
}

int setfps(lua_State* L) {
    WINDOW_NOT_INITIALIZED_CHECK();
    int fps = luaL_checkinteger(L, 1);
    SetTargetFPS(fps);
    return 0;
}

int getfps(lua_State* L) {
    WINDOW_NOT_INITIALIZED_CHECK();
    int fps = GetFPS();
    lua_pushinteger(L, fps);
    return 1;
}

int isfiledropped(lua_State* L) {
    WINDOW_NOT_INITIALIZED_CHECK();
    bool dropped = IsFileDropped();
    lua_pushboolean(L, dropped);
    return 1;
}

int getdroppedfiles(lua_State* L) {
    WINDOW_NOT_INITIALIZED_CHECK();

    FilePathList files = LoadDroppedFiles();
    lua_createtable(L, files.count, 0);

    for (unsigned int i = 0; i < files.count; ++i) {
        lua_pushstring(L, files.paths[i]);
        lua_rawseti(L, -2, i + 1);
    }

    UnloadDroppedFiles(files);

    return 1;
}

int getmousepos(lua_State* L) {
    WINDOW_NOT_INITIALIZED_CHECK();
    Vector2 mousePos = GetMousePosition();
    lua_pushvector(L, mousePos.x, mousePos.y, 0.0f, 0.0f);
    return 1;
}

int getmousescroll(lua_State* L) {
    WINDOW_NOT_INITIALIZED_CHECK();
    Vector2 mouseScroll = GetMouseWheelMoveV();
    lua_pushvector(L, mouseScroll.x, mouseScroll.y, 0.0f, 0.0f);
    return 1;
}

int setfullscreen(lua_State* L) {
    WINDOW_NOT_INITIALIZED_CHECK();
    bool fullscreen = luaL_optboolean(L, 1, true);
    bool isFullscreen = IsWindowFullscreen();
    if (fullscreen != isFullscreen) {
        ToggleFullscreen();
    }
    return 0;
}

int isfullscreen(lua_State* L) {
    WINDOW_NOT_INITIALIZED_CHECK();
    bool isFullscreen = IsWindowFullscreen();
    lua_pushboolean(L, isFullscreen);
    return 1;
}

int getmonitorsize(lua_State* L) {
    WINDOW_NOT_INITIALIZED_CHECK();
    int monitor = GetCurrentMonitor();
    int width = GetMonitorWidth(monitor);
    int height = GetMonitorHeight(monitor);
    lua_pushvector(L, static_cast<float>(width), static_cast<float>(height), 0.0f, 0.0f);    
    return 1;
}

int setsize(lua_State* L) {
    WINDOW_NOT_INITIALIZED_CHECK();
    if (lua_isvector(L, 1)) {
        const float* size = lua_tovector(L, 1);
        SetWindowSize(static_cast<int>(size[0]), static_cast<int>(size[1]));
        return 0;
    }
    int width = luaL_checkinteger(L, 1);
    int height = luaL_checkinteger(L, 2);
    SetWindowSize(width, height);
    
    return 0;
}

int setposition(lua_State* L) {
    WINDOW_NOT_INITIALIZED_CHECK();
    if (lua_isvector(L, 1)) {
        const float* size = lua_tovector(L, 1);
        SetWindowPosition(static_cast<int>(size[0]), static_cast<int>(size[1]));
        return 0;
    }
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    SetWindowPosition(x, y);
    
    return 0;
}

int setstate(lua_State* L) {
    unsigned int flags = static_cast<unsigned int>(luaL_checkinteger(L, 1));
    bool enable = luaL_optboolean(L, 2, true);
    if (enable) {
        if (window::is_window_initialized()) {
            SetWindowState(flags);
        } else {
            SetConfigFlags(flags);
        }
    } else if (window::is_window_initialized()) {
        ClearWindowState(flags);
    }
    return 0;
}

static const std::pair<const char*, ConfigFlags> windowStates[] = {
    { "vsync_hint", FLAG_VSYNC_HINT },
    { "fullscreen_mode", FLAG_FULLSCREEN_MODE },
    { "resizable", FLAG_WINDOW_RESIZABLE },
    { "undecorated", FLAG_WINDOW_UNDECORATED },
    { "hidden", FLAG_WINDOW_HIDDEN },
    { "minimized", FLAG_WINDOW_MINIMIZED },
    { "maximized", FLAG_WINDOW_MAXIMIZED },
    { "unfocused", FLAG_WINDOW_UNFOCUSED },
    { "topmost", FLAG_WINDOW_TOPMOST },
    { "always_run", FLAG_WINDOW_ALWAYS_RUN },
    { "transparent", FLAG_WINDOW_TRANSPARENT },
    { "highdpi", FLAG_WINDOW_HIGHDPI },
    { "mouse_passthrough", FLAG_WINDOW_MOUSE_PASSTHROUGH },
    { "borderless_windowed", FLAG_BORDERLESS_WINDOWED_MODE },
    { "msaa_4x_hint", FLAG_MSAA_4X_HINT },
    { "interlaced_hint", FLAG_INTERLACED_HINT },
};

int noop(lua_State* L) {
    // No operation function
    return 0;
}

} // namespace window


static int window_newindex(lua_State* L) {
    // Make users able to overwrite update() and draw() functions
    const char* key = luaL_checkstring(L, 2);
    if (strcmp(key, "draw") == 0 || strcmp(key, "update") == 0) {
        // Check that the value being set is a function
        luaL_checktype(L, 3, LUA_TFUNCTION);
        lua_setreadonly(L, 1, false);
        lua_setfield(L, 1, key);
        lua_setreadonly(L, 1, true);
        return 0;
    }
    luaL_error(L, "attempt to modify read-only field '%s'", key);
}

int adoreopen_window(lua_State* L)
{
    lua_createtable(L, 0, std::size(window::lib));

    for (auto& [name, func] : window::lib)
    {
        if (!name || !func)
            break;

        lua_pushcfunction(L, func, name);
        lua_setfield(L, -2, name);
    }

    lua_createtable(L, 0, std::size(window::windowStates));
    for (const auto& [name, flag] : window::windowStates) {
        lua_pushinteger(L, static_cast<lua_Integer>(flag));
        lua_setfield(L, -2, name);
    }
    lua_setreadonly(L, -1, true);
    lua_setfield(L, -2, "states");

    lua_createtable(L, 0, 1);
    lua_pushcfunction(L, window_newindex, "__newindex");
    lua_setfield(L, -2, "__newindex");
    lua_setmetatable(L, -2);

    lua_setreadonly(L, -1, false);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "_WINDOW");

    return 1;
}

