#pragma once

#include "lua.h"
#include "lualib.h"

// open the library as a table on top of the stack
int adoreopen_graphics(lua_State* L);

namespace graphics
{

int rectangle(lua_State* L);
int circle(lua_State* L);
int setcolor(lua_State* L);
int setfontsize(lua_State* L);
int print(lua_State* L);
int clearscreen(lua_State* L);

static const luaL_Reg lib[] = {
    {"rectangle", rectangle},
    {"circle", circle},
    {"setcolor", setcolor},
    {"setfontsize", setfontsize},
    {"print", print},
    {"clearscreen", clearscreen},

    {nullptr, nullptr},
};

} // namespace graphics
