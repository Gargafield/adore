#pragma once

#include "lua.h"
#include "lualib.h"

// open the library as a table on top of the stack
int adoreopen_gui(lua_State* L);

namespace gui
{

int button(lua_State* L);
int getstyle(lua_State* L);

static const luaL_Reg lib[] = {
    {"button", button},
    {"getstyle", getstyle},
    {nullptr, nullptr},
};

} // namespace gui
