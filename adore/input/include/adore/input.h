#pragma once

#include "lua.h"
#include "lualib.h"

// open the library as a table on top of the stack
int adoreopen_input(lua_State* L);

namespace input
{

int haspressed(lua_State* L);
int isdown(lua_State* L);

static const luaL_Reg lib[] = {
    {"haspressed", haspressed},
    {"isdown", isdown},
    {nullptr, nullptr}
};

} // namespace input
