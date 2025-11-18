#pragma once

#include "lua.h"
#include "lualib.h"

#include <string>

// open the library as a table on top of the stack
int adoreopen_atem(lua_State* L);

constexpr int kAtemUserdataTag = 96;

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

static const luaL_Reg udata[] = {
    {"close", close},
    {"fadetoblack", fadetoblack},
    {nullptr, nullptr},
};

} // namespace atem
