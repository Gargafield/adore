#pragma once

#include "lua.h"
#include "lualib.h"

#include "raylib.h"

namespace rect
{

int check_rect(lua_State* L, int index, Rectangle* outRect);

} // namespace rect
