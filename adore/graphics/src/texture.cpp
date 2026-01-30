#include "adore/texture.h"

#include "adore/core.h"
#include "adore/window.h"
#include "adore/colors.h"
#include "adore/image.h"
#include <memory>
#include <iostream>
#include "raylib.h"

namespace texture {

int create_texture_userdata(lua_State* L, const Texture2D& texture, bool owned) {
    TextureRef* texPtr = static_cast<TextureRef*>(lua_newuserdatatagged(L, sizeof(TextureRef), kTextureUserdataTag));
    texPtr->texture = texture;
    texPtr->owned = owned;

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

    return create_texture_userdata(L, texture, true);
}

int load_texture_from_image(lua_State* L) {
    WINDOW_NOT_INITIALIZED_CHECK();

    Image* image = image::check_image(L, 1);

    Texture2D texture = LoadTextureFromImage(*image);
    if (texture.id == 0) {
        luaL_error(L, "Failed to load texture from image");
    }

    return create_texture_userdata(L, texture, true);
}

TextureRef* check_texture(lua_State* L, int index) {
    void* ud = lua_touserdatatagged(L, index, kTextureUserdataTag);
    if (!ud) {
        luaL_typeerror(L, index, "Texture");
    }
    return static_cast<TextureRef*>(ud);
}

int index(lua_State* L) {
    TextureRef* textureRef = check_texture(L, 1);
    const char* key = luaL_checkstring(L, 2);

    if (strcmp(key, "width") == 0) {
        lua_pushinteger(L, textureRef->texture.width);
        return 1;
    } else if (strcmp(key, "height") == 0) {
        lua_pushinteger(L, textureRef->texture.height);
        return 1;
    } else if (strcmp(key, "id") == 0) {
        lua_pushinteger(L, textureRef->texture.id);
        return 1;
    } else if (strcmp(key, "mipmaps") == 0) {
        lua_pushinteger(L, textureRef->texture.mipmaps);
        return 1;
    } else if (strcmp(key, "format") == 0) {
        lua_pushinteger(L, textureRef->texture.format);
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

    TextureRef* textureRef = check_texture(L, 1);
    Color color = WHITE;
    if (numargs == 4) {
        color = color::check_color(L, 4);
    } else if (numargs == 6) {
        Color tint = color::check_color(L, 6);
    } else if (numargs != 3) {
        luaL_error(L, "Invalid number of arguments to draw texture");
    }

    if (lua_istable(L, 2) && lua_istable(L, 3)) {
        // Draw with source and destination rectangles
        Rectangle sourceRect;
        Rectangle destRect;

        // Source rectangle
        lua_rawgeti(L, 2, 1);
        sourceRect.x = static_cast<float>(luaL_checknumber(L, -1));
        lua_pop(L, 1);
        lua_rawgeti(L, 2, 2);
        sourceRect.y = static_cast<float>(luaL_checknumber(L, -1));
        lua_pop(L, 1);
        lua_rawgeti(L, 2, 3);
        sourceRect.width = static_cast<float>(luaL_checknumber(L, -1));
        lua_pop(L, 1);
        lua_rawgeti(L, 2, 4);
        sourceRect.height = static_cast<float>(luaL_checknumber(L, -1));
        lua_pop(L, 1);

        // Destination rectangle
        lua_rawgeti(L, 3, 1);
        destRect.x = static_cast<float>(luaL_checknumber(L, -1));
        lua_pop(L, 1);
        lua_rawgeti(L, 3, 2);
        destRect.y = static_cast<float>(luaL_checknumber(L, -1));
        lua_pop(L, 1);
        lua_rawgeti(L, 3, 3);
        destRect.width = static_cast<float>(luaL_checknumber(L, -1));
        lua_pop(L, 1);
        lua_rawgeti(L, 3, 4);
        destRect.height = static_cast<float>(luaL_checknumber(L, -1));
        lua_pop(L, 1);

        DrawTexturePro(textureRef->texture, sourceRect, destRect, {0, 0}, 0, color);
        return 0;
    }

    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);

    if (numargs == 6) {
        float rotation = static_cast<float>(luaL_checknumber(L, 4));
        float scale = static_cast<float>(luaL_checknumber(L, 5));

        Vector2 position = { static_cast<float>(x), static_cast<float>(y) };
        DrawTextureEx(textureRef->texture, position, rotation, scale, color);
        return 0;
    } else {
        DrawTexture(textureRef->texture, x, y, color);
    }

    return 0;
}

int draw_texture_flipped(lua_State* L) {
    int numargs = lua_gettop(L);
    if (numargs < 4) {
        luaL_error(L, "Expected at least 4 arguments (texture, x, y, axis)");
    }

    TextureRef* textureRef = check_texture(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    const char* axis = luaL_checkstring(L, 4);

    Rectangle srcRect = {
        0, 0, textureRef->texture.width, textureRef->texture.height
    };

    if (strcmp(axis, "x") == 0) {
        srcRect.width = -srcRect.width;
    } else if (strcmp(axis, "y") == 0) {
        srcRect.height = -srcRect.height;
    } else if (strcmp(axis, "xy") == 0) {
        srcRect.width = -srcRect.width;
        srcRect.height = -srcRect.height;
    } else {
        luaL_error(L, "Invalid axis value for drawflipped. Expected 'x', 'y', or 'xy'.");
    }

    DrawTextureRec(textureRef->texture, 
        srcRect,
        { static_cast<float>(x), static_cast<float>(y) },
        WHITE
    );

    return 0;
}

int set_filter(lua_State* L) {
    TextureRef* textureRef = check_texture(L, 1);
    int filter = luaL_checkinteger(L, 2);

    SetTextureFilter(textureRef->texture, filter);

    return 0;
}

static const std::pair<const char*, TextureFilter> filterModes[] = {
    { "point", TextureFilter::TEXTURE_FILTER_POINT },
    { "bilinear", TextureFilter::TEXTURE_FILTER_BILINEAR },
    { "trilinear", TextureFilter::TEXTURE_FILTER_TRILINEAR },
    { "anisotropic_4x", TextureFilter::TEXTURE_FILTER_ANISOTROPIC_4X },
    { "anisotropic_8x", TextureFilter::TEXTURE_FILTER_ANISOTROPIC_8X },
    { "anisotropic_16x", TextureFilter::TEXTURE_FILTER_ANISOTROPIC_16X },
};

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
            texture::TextureRef* textureRef = static_cast<texture::TextureRef*>(ud);
            if (textureRef->owned) {
                UnloadTexture(textureRef->texture);
            }
        }
    );

    lua_pop(L, 1);

    lua_createtable(L, 0, std::size(texture::lib));
    luaL_register(L, nullptr, texture::lib); //

    lua_createtable(L, 0, std::size(texture::filterModes));
    for (const auto& pair : texture::filterModes) {
        lua_pushinteger(L, pair.second);
        lua_setfield(L, -2, pair.first);
    }

    lua_setreadonly(L, -1, true);
    lua_setfield(L, -2, "filter");
    
    lua_setreadonly(L, -1, true);

    return 1;
}
