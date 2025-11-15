#pragma once

#include "lua.h"
#include "lualib.h"

#ifndef HYPERDECK_PORT
#define HYPERDECK_PORT 9993
#endif

// open the library as a table on top of the stack
int adoreopen_hyperdeck(lua_State* L);

constexpr int kHyperdeckDeviceUserdataTag = 98;

namespace hyperdeck
{

int connect(lua_State* L);

static const luaL_Reg lib[] = {
    {"connect", connect},
    {nullptr, nullptr},
};

int index(lua_State* L);

int close(lua_State* L);
int send(lua_State* L);

static const luaL_Reg udata[] = {
    {"close", close},
    {"send", send},
    {nullptr, nullptr},
};

} // namespace hyperdeck
