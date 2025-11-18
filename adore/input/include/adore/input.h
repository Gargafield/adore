#pragma once

#include "lua.h"
#include "lualib.h"

// open the library as a table on top of the stack
int adoreopen_input(lua_State* L);

namespace input
{

int ispressed(lua_State* L);

static const luaL_Reg lib[] = {
    {"ispressed", ispressed},
    {nullptr, nullptr}
};

} // namespace input
