#pragma once

#include "lua.h"
#include "lualib.h"

#include "raylib.h"

// open the library as a table on top of the stack
int adoreopen_colors(lua_State* L);

namespace color
{

Color check_color(lua_State* L, int index);

int from(lua_State* L);
int fade(lua_State* L);
int rgb(lua_State* L);

static const luaL_Reg lib[] = {
    {"from", from},
    {"fade", fade},
    {"rgb", rgb},

    {nullptr, nullptr},
};

} // namespace color
