#include "adore/hyperdeck.h"
#include <memory>

#include "BMDSwitcherAPI.tlh"

namespace hyperdeck {




int connect(lua_State* L) {

}

} // namespace hyperdeck

static int adoreregister_hyperdeck(lua_State* L)
{
    luaL_newmetatable(L, "HyperdeckDevice");

    lua_pushvalue(L, -1);
    lua_setuserdatametatable(L, kHyperdeckDeviceUserdataTag);

    lua_pushcfunction(L, hyperdeck::index, "__index");
    lua_setfield(L, -2, "__index");

    luaL_register(L, nullptr, hyperdeck::udata);

    lua_setuserdatadtor(L, kHyperdeckDeviceUserdataTag,
        [](lua_State* L, void* ud)
        {
            hyperdeck::HyperdeckDevice* hyperdeck = static_cast<hyperdeck::HyperdeckDevice*>(ud);
            if (!hyperdeck)
                return;
            if (hyperdeck->socket)
                us_socket_close(0, hyperdeck->socket, 0, 0);
            
            us_loop_free(hyperdeck->loop);
            us_socket_context_free(0, hyperdeck->context);
        }
    );

    lua_setreadonly(L, -1, true);
    lua_pop(L, 1);

    return 0;
}

int adoreopen_hyperdeck(lua_State* L)
{
    lua_createtable(L, 0, std::size(hyperdeck::lib));

    for (auto& [name, func] : hyperdeck::lib)
    {
        if (!name || !func)
            break;

        lua_pushcfunction(L, func, name);
        lua_setfield(L, -2, name);
    }

    lua_setreadonly(L, -1, false);

    return 1;
}

