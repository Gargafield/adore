#pragma once

#include "lua.h"
#include "lualib.h"

#include "raylib.h"

int adoreregister_image(lua_State* L);

namespace image
{

int load_image(lua_State* L);
Image* check_image(lua_State* L, int index);
int create_image_userdata(lua_State* L, const Image& image);
int index(lua_State* L);

int format_image(lua_State* L);
int export_image(lua_State* L);

static const luaL_Reg udata[] = {
    {nullptr, nullptr},
};

static const luaL_Reg lib[] = {
    {"load", load_image},
    {"format", format_image},
    {"export", export_image},
    {nullptr, nullptr},
};

} // namespace color
