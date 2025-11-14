#pragma once

#include "lua.h"
#include "lualib.h"
#include "libusockets.h"

// open the library as a table on top of the stack
int adoreopen_hyperdeck(lua_State* L);

constexpr int kHyperdeckDeviceUserdataTag = 98;

namespace hyperdeck
{

struct HyperdeckDevice {
    lua_State* L;
    struct us_loop_t* loop;
    struct us_socket_t* socket;
    struct us_socket_context_t* context;
};

int connect(lua_State* L);

static const luaL_Reg lib[] = {
    {"connect", connect},
    {nullptr, nullptr},
};

int index(lua_State* L);

static const luaL_Reg udata[] = {
    {nullptr, nullptr},
}

} // namespace hyperdeck
