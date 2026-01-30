#pragma once

#include "lua.h"
#include "lualib.h"

#include <string>

// open the library as a table on top of the stack
int adoreopen_atem(lua_State* L);


namespace atem
{

struct Atem;

Atem* checkatem(lua_State* L, int idx);

int connect(lua_State* L);

static const luaL_Reg lib[] = {
    {"connect", connect},
    {nullptr, nullptr},
};

int close(lua_State* L);
int index(lua_State* L);
int fadetoblack(lua_State* L);
int setpreview(lua_State* L);
int getpreview(lua_State* L);
int setprogram(lua_State* L);
int getprogram(lua_State* L);
int cut(lua_State* L);
int transition(lua_State* L);

static const luaL_Reg udata[] = {
    {"close", close},
    {"fadetoblack", fadetoblack},
    {"setpreview", setpreview},
    {"getpreview", getpreview},
    {"setprogram", setprogram},
    {"getprogram", getprogram},
    {"cut", cut},
    {"transition", transition},
    {nullptr, nullptr},
};

} // namespace atem
