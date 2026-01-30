#pragma once

#include "lua.h"
#include "lualib.h"

#include "adore/texture.h"
#include "adore/rendertexture.h"
#include "adore/image.h"
#include "adore/font.h"


// open the library as a table on top of the stack
int adoreopen_graphics(lua_State* L);

namespace graphics
{

int rectangle(lua_State* L);
int circle(lua_State* L);
int print(lua_State* L);
int clear(lua_State* L);

static const luaL_Reg lib[] = {
    {"rectangle", rectangle},
    {"circle", circle},
    {"print", print},
    {"clear", clear},

    {nullptr, nullptr},
};

static const luaL_Reg modules[] = {
    { "texture", adoreregister_texture },
    { "image", adoreregister_image },
    { "font", adoreregister_font },
    { "rendertexture", adoreregister_rendertexture },

    { nullptr, nullptr }
};


} // namespace graphics
