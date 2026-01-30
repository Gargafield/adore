#include "adore/font.h"

#include "adore/window.h"
#include "adore/colors.h"
#include "adore/texture.h"
#include <memory>
#include <iostream>
#include "raylib.h"

namespace font {

int create_font_userdata(lua_State* L, const Font& font) {
    Font* fontPtr = static_cast<Font*>(lua_newuserdatatagged(L, sizeof(Font), kFontUserdataTag));
    *fontPtr = font;

    lua_getuserdatametatable(L, kFontUserdataTag);
    lua_setmetatable(L, -2);

    return 1;
}

int load_font_from_path(lua_State* L) {
    WINDOW_NOT_INITIALIZED_CHECK();

    const char* path = luaL_checkstring(L, 1);
    int size = luaL_optinteger(L, 2, 32);

    Font font = LoadFontEx(path, size, nullptr, 95);
    if (font.texture.id == 0) {
        luaL_error(L, "Failed to load font: %s", path);
    }

    return create_font_userdata(L, font);
}

Font* check_font(lua_State* L, int index) {
    void* ud = lua_touserdatatagged(L, index, kFontUserdataTag);
    if (!ud) {
        luaL_typeerror(L, index, "Font");
    }
    return static_cast<Font*>(ud);
}

int index(lua_State* L) {
    Font* font = check_font(L, 1);
    const char* key = luaL_checkstring(L, 2);

    if (strcmp(key, "texture") == 0) {
        return texture::create_texture_userdata(L, font->texture, false /* owned */);
    }

    luaL_error(L, "Attempt to access invalid Texture property: %s", key);
    return 0;
}

int draw_font(lua_State* L) {
    int numargs = lua_gettop(L);
    if (numargs < 3) {
        luaL_error(L, "Expected at least 3 arguments (font, x, y, [, rotation, scale, tint])");
    }

    Font* font = check_font(L, 1);
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    int size = luaL_checkinteger(L, 4);
    const char* text = luaL_checkstring(L, 5);
    Color color = color::check_color(L, 6);
    float spacing = luaL_optnumber(L, 7, 1.0f);

    DrawTextEx(*font, text, { x, y }, static_cast<float>(size), spacing, WHITE);

    return 0;
}

int measure_text(lua_State* L) {
    Font* font = check_font(L, 1);
    int size = luaL_checkinteger(L, 2);
    const char* text = luaL_checkstring(L, 3);
    float spacing = luaL_optnumber(L, 4, 1.0f);

    Vector2 dimensions = MeasureTextEx(*font, text, static_cast<float>(size), spacing);

    lua_pushvector(L, dimensions.x, dimensions.y, 0.0f, 0.0f);
    return 1;
}

int get_default_font(lua_State* L) {
    Font defaultFont = GetFontDefault();
    return create_font_userdata(L, defaultFont);
}

} // namespace font


int adoreregister_font(lua_State* L)
{
    luaL_newmetatable(L, "Font");
    lua_pushvalue(L, -1);
    lua_setuserdatametatable(L, kFontUserdataTag);

    lua_pushcfunction(L, font::index, nullptr);
    lua_setfield(L, -2, "__index");

    lua_setreadonly(L, -1, true);

    lua_setuserdatadtor(L, kFontUserdataTag,
        [](lua_State* L, void* ud)
        {
            Font* font = static_cast<Font*>(ud);
            UnloadFont(*font);
        }
    );

    lua_pop(L, 1);

    lua_createtable(L, 0, std::size(font::lib));
    luaL_register(L, nullptr, font::lib); //
    lua_setreadonly(L, -1, true);

    return 1;
}
