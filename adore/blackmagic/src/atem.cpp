#include "adore/atem.h"

#include "BMDSwitcherAPI.tlh"
#include <wrl/client.h>

namespace atem {
    
#define COM_CALL(expr) \
    { \
        HRESULT hr__ = (expr); \
        if (FAILED(hr__)) { \
            luaL_error(L, "COM call failed: " #expr " (HRESULT: 0x%08X)", hr__); \
        } \
    }

static std::string bstr_to_string(BSTR bstr) {
    _bstr_t bstrWrapper(bstr, false);
    const char* cstr = static_cast<const char*>(bstrWrapper);
    ::SysFreeString(bstr);
    return std::string(cstr ? cstr : "");
}

struct Atem {
    Microsoft::WRL::ComPtr<IBMDSwitcher> switcher;
    Microsoft::WRL::ComPtr<IBMDSwitcherMixEffectBlock> effect;
    std::string name;

    Atem()
        : switcher(nullptr)
    { }

    void setup(lua_State* L) {
        auto ptr = switcher.Get();

        BSTR productName;
        COM_CALL(ptr->GetProductName(&productName));

        name = bstr_to_string(productName);

        IBMDSwitcherMixEffectBlockIterator* iterator;
        UUID iid = __uuidof(IBMDSwitcherMixEffectBlockIterator);
        COM_CALL(ptr->CreateIterator(&iid, (void**)&iterator));

        iterator->AddRef();
        IBMDSwitcherMixEffectBlock* mixEffectBlock;
        COM_CALL(iterator->Next(&mixEffectBlock));

        mixEffectBlock->AddRef();
        effect.Attach(mixEffectBlock);
        iterator->Release();
    }

    void close() {
        switcher.Reset();
    }
};

Atem* checkatem(lua_State* L, int idx) {
    auto atemPtr = static_cast<Atem**>(lua_touserdatatagged(L, idx, kAtemUserdataTag));

    if (atemPtr == nullptr || *atemPtr == nullptr) {
        luaL_error(L, "Invalid ATEM userdata");
    }

    return *atemPtr;
}

int connect(lua_State* L) {
    const char* address = luaL_checkstring(L, 1);

    IBMDSwitcherDiscovery* discovery = nullptr;
    CoCreateInstance(
        __uuidof(CBMDSwitcherDiscovery),
        nullptr,
        CLSCTX_ALL,
        __uuidof(IBMDSwitcherDiscovery),
        (void**)&discovery);

    discovery->AddRef();

    IBMDSwitcher* switcher = nullptr;
    _BMDSwitcherConnectToFailure failReason;
    HRESULT result = discovery->ConnectTo(bstr_t(address), &switcher, &failReason);

    if (FAILED(result) || switcher == nullptr) {
        discovery->Release();
        lua_pushnil(L);
        lua_pushfstring(L, "Failed to connect to ATEM switcher at %s (error code: %d)", address, failReason);
        return 2;
    }
    discovery->Release();
    switcher->AddRef();

    auto atem = new Atem();
    atem->switcher.Attach(switcher);
    atem->setup(L);

    Atem** atemPtr = static_cast<Atem**>(lua_newuserdatatagged(L, sizeof(Atem *), kAtemUserdataTag));
    *atemPtr = atem;
    lua_getuserdatametatable(L, kAtemUserdataTag);
    lua_setmetatable(L, -2);
    
    return 1;
}

int close(lua_State* L) {
    Atem* atem = checkatem(L, 1);
    atem->close();
    return 0;
}

int fadetoblack(lua_State* L) {
    Atem* atem = checkatem(L, 1);
    bool enable = luaL_optboolean(L, 2, true);
    auto effect = atem->effect.Get();
    
    long isInBlack = 0;
    COM_CALL(effect->GetInFadeToBlack(&isInBlack));

    if (enable && isInBlack) {
        return 0; // already fully black
    } else if (!enable && !isInBlack) {
        return 0; // already not black
    }

    COM_CALL(effect->PerformFadeToBlack());
    return 0;
}

int index(lua_State* L) {
    const char* key = luaL_checkstring(L, 2);

    for (auto& [name, func] : udata) {
        if (name && strcmp(name, key) == 0) {
            lua_pushcfunction(L, func, name);
            return 1;
        }
    }

    Atem* atem = checkatem(L, 1);

    if (strcmp(key, "name") == 0) {
        lua_pushstring(L, atem->name.c_str());
        return 1;
    }

    lua_pushnil(L);
    return 1;
}

} // namespace atem

static int adoreregister_atem(lua_State* L)
{
    luaL_newmetatable(L, "Atem");

    lua_pushvalue(L, -1);
    lua_setuserdatametatable(L, kAtemUserdataTag);

    lua_pushstring(L, "The metatable is locked");
    lua_setfield(L, -2, "__metatable");

    lua_pushstring(L, "atem");
    lua_setfield(L, -2, "__type");

    lua_pushcfunction(L, atem::index, "__index");
    lua_setfield(L, -2, "__index");

    lua_setreadonly(L, -1, true);
    lua_pop(L, 1);

    lua_setuserdatadtor(L, kAtemUserdataTag, [](lua_State* L, void* userdata) {
        atem::Atem** atem = static_cast<atem::Atem**>(userdata);
        if (atem && *atem) {
            delete *atem;
            *atem = nullptr;
        }
    });

    return 0;
}

int adoreopen_atem(lua_State* L)
{
    lua_createtable(L, 0, std::size(atem::lib));

    for (auto& [name, func] : atem::lib)
    {
        if (!name || !func)
            break;

        lua_pushcfunction(L, func, name);
        lua_setfield(L, -2, name);
    }

    lua_setreadonly(L, -1, true);
    adoreregister_atem(L);

    CoInitialize(nullptr);

    return 1;
}

