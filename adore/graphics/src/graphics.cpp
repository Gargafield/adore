#include "adore/graphics.h"
#include <memory>
#include "raylib.h"
#include <iostream>

namespace graphics {

int rectangle(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    Color color = WHITE;
    DrawRectangle(x, y, width, height, color);
    return 0;
}

} // namespace graphics


int adoreopen_graphics(lua_State* L)
{
    lua_createtable(L, 0, std::size(graphics::lib));

    for (auto& [name, func] : graphics::lib)
    {
        if (!name || !func)
            break;

        lua_pushcfunction(L, func, name);
        lua_setfield(L, -2, name);
    }

    lua_setreadonly(L, -1, true);

    return 1;
}

