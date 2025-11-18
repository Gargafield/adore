#include "adore/atem.h"

#include "BMDSwitcherAPI.tlh"
#include <wrl/client.h>
#include <vector>

namespace atem {

template <typename T>
using COMPTR = Microsoft::WRL::ComPtr<T>;

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

struct AtemInput {
    int64_t id;
    std::string shortName;
    std::string longName;
    _BMDSwitcherPortType type;

    static AtemInput from_com(lua_State* L, IBMDSwitcherInput* input)
    {
        AtemInput atemInput;
        input->AddRef();

        COM_CALL(input->GetInputId(&atemInput.id));

        BSTR sname;
        COM_CALL(input->GetShortName(&sname));
        atemInput.shortName = bstr_to_string(sname);

        BSTR lname;
        COM_CALL(input->GetLongName(&lname));
        atemInput.longName = bstr_to_string(lname);

        COM_CALL(input->GetPortType(&atemInput.type));

        return atemInput;
    }
};

struct Atem {
    COMPTR<IBMDSwitcher> switcher;
    COMPTR<IBMDSwitcherMixEffectBlock> effect;
    std::vector<AtemInput> inputs;
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

        reconcile_inputs(L);
    }

    void reconcile_inputs(lua_State* L) {
        inputs.clear();

        IBMDSwitcherInputIterator* inputIterator;
        UUID iid = __uuidof(IBMDSwitcherInputIterator);
        COM_CALL(switcher->CreateIterator(&iid, (void**)&inputIterator));
        inputIterator->AddRef();

        IBMDSwitcherInput* input;
        while (inputIterator->Next(&input) == S_OK && input != nullptr) {
            inputs.push_back(AtemInput::from_com(L, input));
            input->Release();
        }
        inputIterator->Release();
    }

    AtemInput* find_input(int64_t id) {
        for (auto& input : inputs) {
            if (input.id == id) {
                return &input;
            }
        }
        return nullptr;
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

int64_t checkAtemInputId(lua_State* L, int idx) {
    if (lua_isnumber(L, idx)) {
        return lua_tointeger(L, idx);
    } else if (lua_istable(L, idx)) {
        lua_getfield(L, idx, "id");
        if (!lua_isnumber(L, -1)) {
            luaL_error(L, "Expected 'id' field to be an integer");
        }
        int64_t id = lua_tointeger(L, -1);
        lua_pop(L, 1);
        return id;
    } else {
        luaL_error(L, "Expected integer or table for ATEM input identifier");
    }
}

static const std::pair<_BMDSwitcherPortType, const char*> kPortTypeNames[] = {
    { _BMDSwitcherPortType::bmdSwitcherPortTypeExternal, "external" },
    { _BMDSwitcherPortType::bmdSwitcherPortTypeBlack, "black" },
    { _BMDSwitcherPortType::bmdSwitcherPortTypeColorBars, "colorbars" },
    { _BMDSwitcherPortType::bmdSwitcherPortTypeColorGenerator, "colorgenerator" },
    { _BMDSwitcherPortType::bmdSwitcherPortTypeMediaPlayerFill, "mediaplayerfill" },
    { _BMDSwitcherPortType::bmdSwitcherPortTypeMediaPlayerCut, "mediaplayercut" },
    { _BMDSwitcherPortType::bmdSwitcherPortTypeSuperSource, "supersource" },
    { _BMDSwitcherPortType::bmdSwitcherPortTypeMixEffectBlockOutput, "mixeffectblockoutput" },
    { _BMDSwitcherPortType::bmdSwitcherPortTypeAuxOutput, "auxoutput" },
    { _BMDSwitcherPortType::bmdSwitcherPortTypeKeyCutOutput, "keycutoutput" },
    { _BMDSwitcherPortType::bmdSwitcherPortTypeMultiview, "multiview" },
    { _BMDSwitcherPortType::bmdSwitcherPortTypeAudioMonitor, "audiomonitor" },
    { _BMDSwitcherPortType::bmdSwitcherPortTypeExternalDirect, "externaldirect" }, 
};

int createAtemInput(lua_State* L, const AtemInput& input) {
    lua_createtable(L, 0, 3);
    lua_pushinteger(L, input.id);
    lua_setfield(L, -2, "id");
    lua_pushstring(L, input.shortName.c_str());
    lua_setfield(L, -2, "shortname");
    lua_pushstring(L, input.longName.c_str());
    lua_setfield(L, -2, "name");
    const char* typeName = "unknown";
    for (const auto& [type, name] : kPortTypeNames) {
        if (type == input.type) {
            typeName = name;
            break;
        }
    }
    lua_pushstring(L, typeName);
    lua_setfield(L, -2, "type");
    return 1;
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

int setpreview(lua_State* L) {
    Atem* atem = checkatem(L, 1);
    int64_t inputId = checkAtemInputId(L, 2);
    auto effect = atem->effect.Get();
    COM_CALL(effect->SetPreviewInput(inputId));
    return 0;
}

int getpreview(lua_State* L) {
    Atem* atem = checkatem(L, 1);
    auto effect = atem->effect.Get();

    int64_t inputId = 0;
    COM_CALL(effect->GetPreviewInput(&inputId));
    auto input = atem->find_input(inputId);
    if (input == nullptr) {
        lua_pushnil(L);
        return 1;
    }
    return createAtemInput(L, *input);
}

int setprogram(lua_State* L) {
    Atem* atem = checkatem(L, 1);
    int64_t inputId = checkAtemInputId(L, 2);
    auto effect = atem->effect.Get();

    COM_CALL(effect->SetProgramInput(inputId));
    return 0;
}

int getprogram(lua_State* L) {
    Atem* atem = checkatem(L, 1);
    auto effect = atem->effect.Get();

    int64_t inputId = 0;
    COM_CALL(effect->GetProgramInput(&inputId));
    auto input = atem->find_input(inputId);
    if (input == nullptr) {
        lua_pushnil(L);
        return 1;
    }
    return createAtemInput(L, *input);
}

int cut(lua_State* L) {
    Atem* atem = checkatem(L, 1);
    auto effect = atem->effect.Get();
    if (lua_gettop(L) > 1) {
        auto next = checkAtemInputId(L, 2);
        COM_CALL(effect->SetPreviewInput(next));
    }
    COM_CALL(effect->PerformCut());
    return 0;
}

int transition(lua_State* L) {
    Atem* atem = checkatem(L, 1);
    auto effect = atem->effect.Get();
    if (lua_gettop(L) > 1) {
        auto next = checkAtemInputId(L, 2);
        COM_CALL(effect->SetPreviewInput(next));
    }
    COM_CALL(effect->PerformAutoTransition());

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
    } else if (strcmp(key, "sources") == 0) {
        lua_createtable(L, atem->inputs.size(), 0);
        for (size_t i = 0; i < atem->inputs.size(); ++i) {
            const AtemInput& input = atem->inputs[i];
            createAtemInput(L, input);
            lua_rawseti(L, -2, i + 1);
        }
        return 1;
    } else if (strcmp(key, "input") == 0) {
        lua_createtable(L, 4, 0);
        for (const auto& input : atem->inputs) {
            if (input.type != _BMDSwitcherPortType::bmdSwitcherPortTypeExternal) {
                continue;
            }
            createAtemInput(L, input);
            lua_rawseti(L, -2, input.id);
        }
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

