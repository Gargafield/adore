#pragma once

#include "lua.h"
#include "lualib.h"

// open the library as a table on top of the stack
int adoreopen_graphics(lua_State* L);

namespace graphics
{

int rectangle(lua_State* L);

static const luaL_Reg lib[] = {
    {"rectangle", rectangle},
    {nullptr, nullptr},
};

} // namespace graphics
