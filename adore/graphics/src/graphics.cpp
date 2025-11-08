#include "adore/graphics.h"
#include <memory>
#include "raylib.h"
#include <iostream>

namespace graphics {

static Color CURRENT_COLOR = WHITE;
static int FONT_SIZE = 20;

int rectangle(lua_State* L) {
    const char* mode = luaL_checkstring(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int width = luaL_checkinteger(L, 4);
    int height = luaL_checkinteger(L, 5);

    if (strcmp(mode, "fill") == 0) {
        DrawRectangle(x, y, width, height, CURRENT_COLOR);
    } else if (strcmp(mode, "line") == 0) {
        DrawRectangleLines(x, y, width, height, CURRENT_COLOR);
    } else {
        luaL_error(L, "Invalid mode for rectangle: %s", mode);
    }

    return 0;
}

int circle(lua_State* L) {
    const char* mode = luaL_checkstring(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int radius = luaL_checkinteger(L, 4);

    if (strcmp(mode, "fill") == 0) {
        DrawCircle(x, y, radius, CURRENT_COLOR);
    } else if (strcmp(mode, "line") == 0) {
        DrawCircleLines(x, y, radius, CURRENT_COLOR);
    } else {
        luaL_error(L, "Invalid mode for circle: %s", mode);
    }

    return 0;
}

int setcolor(lua_State* L) {
    if (lua_gettop(L) == 0) {
        CURRENT_COLOR = WHITE;
        return 0;
    }

    const float* vector = lua_tovector(L, 1);
    if (vector != NULL) {
        CURRENT_COLOR = Color{
            static_cast<unsigned char>(vector[0]),
            static_cast<unsigned char>(vector[1]),
            static_cast<unsigned char>(vector[2]),
            255
        };
        return 0;
    } else {
        int r = luaL_checkinteger(L, 1);
        int g = luaL_checkinteger(L, 2);
        int b = luaL_checkinteger(L, 3);
    
        CURRENT_COLOR = Color{
            static_cast<unsigned char>(r),
            static_cast<unsigned char>(g),
            static_cast<unsigned char>(b),
            255
        };
    }


    return 0;
}


int setfontsize(lua_State* L) {
    int size = luaL_checkinteger(L, 1);

    FONT_SIZE = size;

    return 0;
}

int print(lua_State* L) {
    const char* text = luaL_checkstring(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);

    DrawText(text, x, y, FONT_SIZE, CURRENT_COLOR);

    return 0;
}

int clearscreen(lua_State* L) {
    ClearBackground(CURRENT_COLOR);

    return 0;
}

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


} // namespace graphics


int adoreopen_graphics(lua_State* L)
{
    lua_createtable(L, 0, std::size(graphics::lib));
    
    // Library functions
    for (auto& [name, func] : graphics::lib)
    {
        if (!name || !func)
            break;

        lua_pushcfunction(L, func, name);
        lua_setfield(L, -2, name);
    }

    // Colors
    lua_createtable(L, 0, std::size(graphics::colors));
    for (auto& [name, color] : graphics::colors) {
        lua_pushvector(L, color.r, color.g, color.b);
        lua_setfield(L, -2, name);
    }

    lua_setreadonly(L, -1, true);
    lua_setfield(L, -2, "colors");

    lua_setreadonly(L, -1, true);

    return 1;
}

