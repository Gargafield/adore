#include "adore/rendertexture.h"

#include "adore/texture.h"
#include "adore/core.h"
#include "adore/window.h"
#include "adore/colors.h"
#include "adore/image.h"
#include <memory>
#include <iostream>
#include "raylib.h"

namespace rendertexture {

int create_rendertexture_userdata(lua_State* L, const RenderTexture& rendertexture) {
    RenderTexture* rtPtr = static_cast<RenderTexture*>(lua_newuserdatatagged(L, sizeof(RenderTexture), kRenderTextureUserdataTag));
    *rtPtr = rendertexture;
    lua_getuserdatametatable(L, kRenderTextureUserdataTag);
    lua_setmetatable(L, -2);

    return 1;
}

int create(lua_State* L) {
    WINDOW_NOT_INITIALIZED_CHECK();

    int width = luaL_checkinteger(L, 1);
    int height = luaL_checkinteger(L, 2);

    RenderTexture rendertexture = LoadRenderTexture(width, height);

    return create_rendertexture_userdata(L, rendertexture);
}

RenderTexture* check_rendertexture(lua_State* L, int index) {
    void* ud = lua_touserdatatagged(L, index, kRenderTextureUserdataTag);
    if (!ud) {
        luaL_typeerror(L, index, "RenderTexture");
    }
    return static_cast<RenderTexture*>(ud);
}

int index(lua_State* L) {
    RenderTexture* rendertexture = check_rendertexture(L, 1);
    const char* key = luaL_checkstring(L, 2);

    if (strcmp(key, "width") == 0) {
        lua_pushinteger(L, rendertexture->texture.width);
        return 1;
    } else if (strcmp(key, "height") == 0) {
        lua_pushinteger(L, rendertexture->texture.height);
        return 1;
    } else if (strcmp(key, "id") == 0) {
        lua_pushinteger(L, rendertexture->id);
        return 1;
    } else if (strcmp(key, "texture") == 0) {
        return texture::create_texture_userdata(L, rendertexture->texture, false);
    } else if (strcmp(key, "depth") == 0) {
        return texture::create_texture_userdata(L, rendertexture->depth, false);
    }

    luaL_error(L, "Attempt to access invalid Texture property: %s", key);
    return 0;
}

int start(lua_State* L) {
    WINDOW_NOT_INITIALIZED_CHECK();

    RenderTexture* rendertexture = check_rendertexture(L, 1);

    BeginTextureMode(*rendertexture);

    return 0;
}

int stop(lua_State* L) {
    WINDOW_NOT_INITIALIZED_CHECK();

    EndTextureMode();

    return 0;
}

} // namespace rendertexture


int adoreregister_rendertexture(lua_State* L)
{
    luaL_newmetatable(L, "RenderTexture");
    lua_pushvalue(L, -1);
    lua_setuserdatametatable(L, kRenderTextureUserdataTag);
    lua_pushcfunction(L, rendertexture::index, nullptr);
    lua_setfield(L, -2, "__index");

    lua_setreadonly(L, -1, true);

    lua_setuserdatadtor(L, kRenderTextureUserdataTag,
        [](lua_State* L, void* ud)
        {
            RenderTexture* rendertexture = static_cast<RenderTexture*>(ud);
            UnloadRenderTexture(*rendertexture);
        }
    );

    lua_pop(L, 1);

    lua_createtable(L, 0, std::size(rendertexture::lib));
    luaL_register(L, nullptr, rendertexture::lib); //
    
    lua_setreadonly(L, -1, true);

    return 1;
}
