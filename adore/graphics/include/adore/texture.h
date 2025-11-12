#pragma once

#include "lua.h"
#include "lualib.h"

#include "raylib.h"

// open the library as a table on top of the stack
int adoreregister_texture(lua_State* L);

constexpr int kTextureUserdataTag = 100;

namespace texture
{

int load_texture(lua_State* L);
Texture2D* check_texture(lua_State* L, int index);
int create_texture_userdata(lua_State* L, const Texture2D& texture);
int index(lua_State* L);

static const luaL_Reg udata[] = {
    {nullptr, nullptr},
};

} // namespace color
