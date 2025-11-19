#pragma once

#include "lua.h"
#include "lualib.h"

#include "adore/texture.h"
#include "adore/image.h"
#include "adore/font.h"


// open the library as a table on top of the stack
int adoreopen_graphics(lua_State* L);

namespace graphics
{

int rectangle(lua_State* L);
int circle(lua_State* L);
int print(lua_State* L);
int clearscreen(lua_State* L);

static const luaL_Reg lib[] = {
    {"rectangle", rectangle},
    {"circle", circle},
    {"print", print},
    {"clearscreen", clearscreen},

    {nullptr, nullptr},
};

static const luaL_Reg modules[] = {
    { "texture", adoreregister_texture },
    { "image", adoreregister_image },
    { "font", adoreregister_font },
    { nullptr, nullptr }
};


} // namespace graphics
