#pragma once

#include "lua.h"
#include "lualib.h"

#include "raylib.h"

int adoreregister_font(lua_State* L);

namespace font
{

int load_font_from_path(lua_State* L);
Font* check_font(lua_State* L, int index);
int create_font_userdata(lua_State* L, const Font& font);
int index(lua_State* L);
int draw_font(lua_State* L);
int measure_text(lua_State* L);
int get_default_font(lua_State* L);

static const luaL_Reg udata[] = {
    {nullptr, nullptr},
};

static const luaL_Reg lib[] = {
    { "load", load_font_from_path },
    { "draw", draw_font },
    { "measure", measure_text }, 
    { "getdefault", get_default_font },
    {nullptr, nullptr},
};

} // namespace color
