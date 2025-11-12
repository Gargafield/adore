#include "adore/blackmagic.h"
#include <memory>

#include "BMDSwitcherAPI.tlh"

namespace blackmagic {

} // namespace blackmagic


int adoreopen_blackmagic(lua_State* L)
{
    lua_createtable(L, 0, std::size(blackmagic::lib));

    for (auto& [name, func] : blackmagic::lib)
    {
        if (!name || !func)
            break;

        lua_pushcfunction(L, func, name);
        lua_setfield(L, -2, name);
    }

    lua_setreadonly(L, -1, false);

    return 1;
}

