#pragma once

#include "lua.h"
#include "lualib.h"

// open the library as a table on top of the stack
int adoreopen_window(lua_State* L);

#define WINDOW_NOT_INITIALIZED_CHECK() \
    if (!window::is_window_initialized()) { \
        luaL_errorL(L, "Window not initialized"); \
    }

namespace window
{

bool is_window_initialized();

int init(lua_State* L);
int setfps(lua_State* L);
int getfps(lua_State* L);
int isfiledropped(lua_State* L);
int getdroppedfiles(lua_State* L);
int getmousepos(lua_State* L);
int getmousescroll(lua_State* L);
int setfullscreen(lua_State* L);
int isfullscreen(lua_State* L);


int noop(lua_State* L);

static const luaL_Reg lib[] = {
    {"init", init},
    {"setfps", setfps},
    {"getfps", getfps},
    {"update", noop},
    {"draw", noop},
    {"isfiledropped", isfiledropped},
    {"getdroppedfiles", getdroppedfiles},
    {"getmousepos", getmousepos},
    {"getmousescroll", getmousescroll},
    {"setfullscreen", setfullscreen},
    {"isfullscreen", isfullscreen},
    {nullptr, nullptr}
};

} // namespace window
