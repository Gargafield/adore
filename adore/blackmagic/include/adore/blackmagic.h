#pragma once

#include "lua.h"
#include "lualib.h"

// open the library as a table on top of the stack
int adoreopen_blackmagic(lua_State* L);

namespace blackmagic
{

static const luaL_Reg lib[] = {
    {nullptr, nullptr},
};

} // namespace blackmagic
