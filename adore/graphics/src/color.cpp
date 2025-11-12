#include "adore/color.h"
#include <memory>
#include <iostream>

#include "raylib.h"

namespace color {

static const std::pair<const char*, Color> colors[] = {
    {"white", WHITE},
    {"black", BLACK},
    {"red", RED},
    {"green", GREEN},
    {"blue", BLUE},
    {"yellow", YELLOW},
    {"orange", ORANGE},
    {"pink", PINK},
    {"purple", PURPLE},
    {"maroon", MAROON},
    {"lime", LIME},
    {"skyblue", SKYBLUE},
    {"violet", VIOLET},
    {"darkgray", DARKGRAY},
    {"gray", GRAY},
    {"lightgray", LIGHTGRAY},
    {"darkblue", DARKBLUE},
    {"darkgreen", DARKGREEN},
    {"brown", BROWN},
    {"darkbrown", DARKBROWN},
    {"raywhite", RAYWHITE},
};

Color check_color(lua_State* L, int index) {
    if (lua_isvector(L, index)) {
        const float* vec = luaL_checkvector(L, index);
        return Color{
            static_cast<unsigned char>(vec[0]),
            static_cast<unsigned char>(vec[1]),
            static_cast<unsigned char>(vec[2]),
            255
        };
    } else {
        luaL_error(L, "Expected a vector for color");
    }
}

int from(lua_State* L) {
    if (lua_isnumber(L, 1)) {
        int num = luaL_checkinteger(L, 1);
        Color color = GetColor(num);
        lua_pushvector(L, color.r, color.g, color.b);
        return 1;
    } else if (lua_isstring(L, 1)) {
        const char* str = luaL_checkstring(L, 1);
        for (auto& [name, color] : colors) {
            if (strcmp(name, str) == 0) {
                lua_pushvector(L, color.r, color.g, color.b);
                return 1;
            }
        }
        luaL_error(L, "Unknown color name: %s", str);
    }

    return 0;
}

} // namespace color


int adoreopen_color(lua_State* L)
{
    lua_createtable(L, 0, std::size(color::lib) + std::size(color::colors));
    
    // Library functions
    for (auto& [name, func] : color::lib)
    {
        if (!name || !func)
            break;

        lua_pushcfunction(L, func, name);
        lua_setfield(L, -2, name);
    }

    // Colors
    for (auto& [name, color] : color::colors) {
        lua_pushvector(L, color.r, color.g, color.b);
        lua_setfield(L, -2, name);
    }

    lua_setreadonly(L, -1, true);

    return 1;
}
