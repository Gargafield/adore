#include "adore/rect.h"
#include <memory>
#include <iostream>

namespace rect {

int check_rect(lua_State* L, int index, Rectangle* outRect) {
    switch (lua_type(L, index))
    {
    case LUA_TTABLE:
        // array of numbers
        lua_rawgeti(L, index, 1);
        outRect->x = static_cast<float>(luaL_checknumber(L, -1));
        lua_pop(L, 1);
        lua_rawgeti(L, index, 2);
        outRect->y = static_cast<float>(luaL_checknumber(L, -1));
        lua_pop(L, 1);
        lua_rawgeti(L, index, 3);
        outRect->width = static_cast<float>(luaL_checknumber(L, -1));
        lua_pop(L, 1);
        lua_rawgeti(L, index, 4);
        outRect->height = static_cast<float>(luaL_checknumber(L, -1));
        lua_pop(L, 1);
        return index + 1;
    case LUA_TNUMBER:
        outRect->x = static_cast<float>(luaL_checknumber(L, index));
        outRect->y = static_cast<float>(luaL_checknumber(L, index + 1));
        outRect->width = static_cast<float>(luaL_checknumber(L, index + 2));
        outRect->height = static_cast<float>(luaL_checknumber(L, index + 3));    
        return index + 4;
    default:
        luaL_error(L, "Expected table or four numbers for rectangle");
        return 0;
    }
}

} // namespace rect

