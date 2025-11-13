#include "adore/texture.h"

#include "adore/window.h"
#include "adore/color.h"
#include "adore/image.h"
#include <memory>
#include <iostream>
#include "raylib.h"

namespace texture {

int create_texture_userdata(lua_State* L, const Texture2D& texture) {
    Texture2D* texPtr = static_cast<Texture2D*>(lua_newuserdatatagged(L, sizeof(Texture2D), kTextureUserdataTag));
    *texPtr = texture;

    lua_getuserdatametatable(L, kTextureUserdataTag);
    lua_setmetatable(L, -2);

    return 1;
}

int load_texture_from_path(lua_State* L) {
    WINDOW_NOT_INITIALIZED_CHECK();

    const char* path = luaL_checkstring(L, 1);

    Texture2D texture = LoadTexture(path);
    if (texture.id == 0) {
        luaL_error(L, "Failed to load texture: %s", path);
    }

    return create_texture_userdata(L, texture);
}

int load_texture_from_image(lua_State* L) {
    WINDOW_NOT_INITIALIZED_CHECK();

    Image* image = image::check_image(L, 1);

    Texture2D texture = LoadTextureFromImage(*image);
    if (texture.id == 0) {
        luaL_error(L, "Failed to load texture from image");
    }

    return create_texture_userdata(L, texture);
}

Texture2D* check_texture(lua_State* L, int index) {
    void* ud = lua_touserdatatagged(L, index, kTextureUserdataTag);
    if (!ud) {
        luaL_typeerror(L, index, "Texture");
    }
    return static_cast<Texture2D*>(ud);
}

int index(lua_State* L) {
    Texture2D* texture = check_texture(L, 1);
    const char* key = luaL_checkstring(L, 2);

    if (strcmp(key, "width") == 0) {
        lua_pushinteger(L, texture->width);
        return 1;
    } else if (strcmp(key, "height") == 0) {
        lua_pushinteger(L, texture->height);
        return 1;
    } else if (strcmp(key, "id") == 0) {
        lua_pushinteger(L, texture->id);
        return 1;
    } else if (strcmp(key, "mipmaps") == 0) {
        lua_pushinteger(L, texture->mipmaps);
        return 1;
    } else if (strcmp(key, "format") == 0) {
        lua_pushinteger(L, texture->format);
        return 1;
    }

    luaL_error(L, "Attempt to access invalid Texture property: %s", key);
    return 0;
}

int draw_texture(lua_State* L) {
    int numargs = lua_gettop(L);
    if (numargs < 3) {
        luaL_error(L, "Expected at least 3 arguments (texture, x, y, [, rotation, scale, tint])");
    }

    Texture2D* texture = check_texture(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);

    if (numargs == 4) {
        Color tint = color::check_color(L, 4);
        DrawTexture(*texture, x, y, tint);
        return 0;
    } else if (numargs == 6) {
        float rotation = static_cast<float>(luaL_checknumber(L, 4));
        float scale = static_cast<float>(luaL_checknumber(L, 5));
        Color tint = color::check_color(L, 6);

        Vector2 position = { static_cast<float>(x), static_cast<float>(y) };
        DrawTextureEx(*texture, position, rotation, scale, tint);
        return 0;
    } else {
        luaL_error(L, "Invalid number of arguments to draw texture");
    }

    return 0;
}

} // namespace texture


int adoreregister_texture(lua_State* L)
{
    luaL_newmetatable(L, "Texture");
    lua_pushvalue(L, -1);
    lua_setuserdatametatable(L, kTextureUserdataTag);

    lua_pushcfunction(L, texture::index, nullptr);
    lua_setfield(L, -2, "__index");

    lua_setreadonly(L, -1, true);

    lua_setuserdatadtor(L, kTextureUserdataTag,
        [](lua_State* L, void* ud)
        {
            Texture2D* texture = static_cast<Texture2D*>(ud);
            UnloadTexture(*texture);
        }
    );

    lua_pop(L, 1);

    lua_newtable(L, 0, std::size(texture::lib));
    luaL_register(L, nullptr, texture::lib); //
    lua_setreadonly(L, -1, true);

    return 1;
}
