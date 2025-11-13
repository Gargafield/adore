#pragma once

#include "lua.h"
#include "lualib.h"

// open the library as a table on top of the stack
int adoreopen_gui(lua_State* L);

namespace gui
{

int button(lua_State* L);
int windowbox(lua_State* L);
int label(lua_State* L);
int combobox(lua_State* L);
int textbox(lua_State* L);

int getstyle(lua_State* L);
int enable(lua_State* L);
int disable(lua_State* L);


static const luaL_Reg lib[] = {
    {"button", button},
    {"windowbox", windowbox},
    {"label", label},
    {"combobox", combobox},
    {"textbox", textbox},
    
    {"getstyle", getstyle},
    {"enable", enable},
    {"disable", disable},
    

    {nullptr, nullptr},
};

} // namespace gui
