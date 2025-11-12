#include "adore/gui.h"
#include <memory>
#include <iostream>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

namespace gui {

int button(lua_State* L)
{
    const float x = luaL_checknumber(L, 1);
    const float y = luaL_checknumber(L, 2);
    const float width = luaL_checknumber(L, 3);
    const float height = luaL_checknumber(L, 4);
    const char* text = luaL_checkstring(L, 5);

    bool result = GuiLabelButton(
        Rectangle{x, y, width, height},
        text
    );
    lua_pushboolean(L, result);
    return 1;
}

int getstyle(lua_State* L)
{
    const int control = static_cast<int>(luaL_checknumber(L, 1));
    const int property = static_cast<int>(luaL_checknumber(L, 2));

    int value = GuiGetStyle(control, property);
    lua_pushnumber(L, value);
    return 1;

}

} // namespace gui


int adoreopen_gui(lua_State* L)
{
    lua_createtable(L, 0, std::size(gui::lib));
    
    // Library functions
    for (auto& [name, func] : gui::lib)
    {
        if (!name || !func)
            break;

        lua_pushcfunction(L, func, name);
        lua_setfield(L, -2, name);
    }

    lua_setreadonly(L, -1, true);

    return 1;
}

