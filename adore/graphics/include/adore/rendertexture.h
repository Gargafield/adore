#pragma once

#include "lua.h"
#include "lualib.h"

#include "raylib.h"

int adoreregister_rendertexture(lua_State* L);

namespace rendertexture
{

int create(lua_State* L);
int start(lua_State* L);
int stop(lua_State* L);

static const luaL_Reg lib[] = {
    { "create", create },
    { "start", start },
    { "stop", stop },

    {nullptr, nullptr},
};

} // namespace color
