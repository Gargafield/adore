#include "adore/graphics.h"

#include "adore/colors.h"
#include "adore/rect.h"
#include <memory>
#include "raylib.h"
#include <iostream>

namespace graphics {

int rectangle(lua_State* L) {
    const char* mode = luaL_checkstring(L, 1);
    Rectangle rect;
    int end = rect::check_rect(L, 2, &rect);
    Color color = color::check_color(L, end);

    if (strcmp(mode, "fill") == 0) {
        DrawRectangle(rect.x, rect.y, rect.width, rect.height, color);
    } else if (strcmp(mode, "line") == 0) {
        DrawRectangleLines(rect.x, rect.y, rect.width, rect.height, color);
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
    Color color = color::check_color(L, 5);

    if (strcmp(mode, "fill") == 0) {
        DrawCircle(x, y, radius, color);
    } else if (strcmp(mode, "line") == 0) {
        DrawCircleLines(x, y, radius, color);
    } else {
        luaL_error(L, "Invalid mode for circle: %s", mode);
    }

    return 0;
}

int print(lua_State* L) {
    const char* text = luaL_checkstring(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int fontsize = luaL_checkinteger(L, 4);
    Color color = color::check_color(L, 5);

    DrawText(text, x, y, fontsize, color);

    return 0;
}

int clearscreen(lua_State* L) {
    Color color = color::check_color(L, 1);

    ClearBackground(color);

    return 0;
}

int drawtexture(lua_State* L) {
    Texture2D* texture = texture::check_texture(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    Color tint = color::check_color(L, 4);

    DrawTexture(*texture, x, y, tint);

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

    // Submodules
    for (auto& [name, func] : graphics::modules)
    {
        if (!name || !func)
            break;

        func(L);
        lua_setfield(L, -2, name);
    }

    lua_setreadonly(L, -1, true);

    return 1;
}

