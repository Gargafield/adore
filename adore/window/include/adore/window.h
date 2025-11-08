#pragma once

#include "lua.h"
#include "lualib.h"

// open the library as a table on top of the stack
int adoreopen_window(lua_State* L);

namespace window
{

int init(lua_State* L);
int setfps(lua_State* L);
int getfps(lua_State* L);
int noop(lua_State* L);

static const luaL_Reg lib[] = {
    {"init", init},
    {"setfps", setfps},
    {"getfps", getfps},
    {"update", noop},
    {"draw", noop},
    {nullptr, nullptr},
};

} // namespace window
