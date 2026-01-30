#pragma once

#include "lua.h"
#include "lualib.h"

#include "raylib.h"

int adoreregister_texture(lua_State* L);

constexpr int kTextureUserdataTag = 100;

namespace texture
{

struct TextureRef {
    Texture2D texture;
    bool owned;
};

int load_texture_from_path(lua_State* L);
int load_texture_from_image(lua_State* L);
TextureRef* check_texture(lua_State* L, int index);
int create_texture_userdata(lua_State* L, const Texture2D& texture, bool owned = true);
int index(lua_State* L);
int draw_texture(lua_State* L);
int set_filter(lua_State* L);

static const luaL_Reg udata[] = {
    {nullptr, nullptr},
};

static const luaL_Reg lib[] = {
    { "load", load_texture_from_path },
    { "fromimage", load_texture_from_image },
    { "draw", draw_texture },
    { "setfilter", set_filter },
    {nullptr, nullptr},
};

} // namespace color
