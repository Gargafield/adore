#include "adore/window.h"
#include <memory>
#include "raylib.h"
#include <iostream>

namespace window {

int init(lua_State* L) {
    int width = luaL_checkinteger(L, 1);
    int height = luaL_checkinteger(L, 2);
    const char* title = luaL_checkstring(L, 3);
    InitWindow(width, height, title);
    return 0;
}

int noop(lua_State* L) {
    // No operation function
    return 0;
}

} // namespace window


static int window_newindex(lua_State* L) {
    // Make users able to overwrite update() and draw() functions
    const char* key = luaL_checkstring(L, 2);
    if (strcmp(key, "draw") == 0) {
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

    lua_createtable(L, 0, 1);
    lua_pushcfunction(L, window_newindex, "__newindex");
    lua_setfield(L, -2, "__newindex");
    lua_setmetatable(L, -2);

    lua_setreadonly(L, -1, false);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "_WINDOW");

    return 1;
}

