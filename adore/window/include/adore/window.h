#pragma once

#include "lua.h"
#include "lualib.h"

// open the library as a table on top of the stack
int adoreopen_window(lua_State* L);

namespace window
{

int init(lua_State* L);

int noop(lua_State* L);

static const luaL_Reg lib[] = {
    {"init", init},
    {"draw", noop},
    {nullptr, nullptr},
};

} // namespace window
