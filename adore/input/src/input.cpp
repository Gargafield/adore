#include "adore/input.h"
#include <memory>
#include "raylib.h"
#include <iostream>

namespace input {

static const std::pair<const char*, KeyboardKey> kKeysMap[] = {
    { "null", KeyboardKey::KEY_NULL },
    { "apostrophe", KeyboardKey::KEY_APOSTROPHE },
    { "comma", KeyboardKey::KEY_COMMA },
    { "minus", KeyboardKey::KEY_MINUS },
    { "period", KeyboardKey::KEY_PERIOD },
    { "slash", KeyboardKey::KEY_SLASH },
    { "zero", KeyboardKey::KEY_ZERO },
    { "one", KeyboardKey::KEY_ONE },
    { "two", KeyboardKey::KEY_TWO },
    { "three", KeyboardKey::KEY_THREE },
    { "four", KeyboardKey::KEY_FOUR },
    { "five", KeyboardKey::KEY_FIVE },
    { "six", KeyboardKey::KEY_SIX },
    { "seven", KeyboardKey::KEY_SEVEN },
    { "eight", KeyboardKey::KEY_EIGHT },
    { "nine", KeyboardKey::KEY_NINE },
    { "semicolon", KeyboardKey::KEY_SEMICOLON },
    { "equal", KeyboardKey::KEY_EQUAL },
    { "a", KeyboardKey::KEY_A },
    { "b", KeyboardKey::KEY_B },
    { "c", KeyboardKey::KEY_C },
    { "d", KeyboardKey::KEY_D },
    { "e", KeyboardKey::KEY_E },
    { "f", KeyboardKey::KEY_F },
    { "g", KeyboardKey::KEY_G },
    { "h", KeyboardKey::KEY_H },
    { "i", KeyboardKey::KEY_I },
    { "j", KeyboardKey::KEY_J },
    { "k", KeyboardKey::KEY_K },
    { "l", KeyboardKey::KEY_L },
    { "m", KeyboardKey::KEY_M },
    { "n", KeyboardKey::KEY_N },
    { "o", KeyboardKey::KEY_O },
    { "p", KeyboardKey::KEY_P },
    { "q", KeyboardKey::KEY_Q },
    { "r", KeyboardKey::KEY_R },
    { "s", KeyboardKey::KEY_S },
    { "t", KeyboardKey::KEY_T },
    { "u", KeyboardKey::KEY_U },
    { "v", KeyboardKey::KEY_V },
    { "w", KeyboardKey::KEY_W },
    { "x", KeyboardKey::KEY_X },
    { "y", KeyboardKey::KEY_Y },
    { "z", KeyboardKey::KEY_Z },
    { "left_bracket", KeyboardKey::KEY_LEFT_BRACKET },
    { "backslash", KeyboardKey::KEY_BACKSLASH },
    { "right_bracket", KeyboardKey::KEY_RIGHT_BRACKET },
    { "grave", KeyboardKey::KEY_GRAVE },
    { "space", KeyboardKey::KEY_SPACE },
    { "escape", KeyboardKey::KEY_ESCAPE },
    { "enter", KeyboardKey::KEY_ENTER },
    { "tab", KeyboardKey::KEY_TAB },
    { "backspace", KeyboardKey::KEY_BACKSPACE },
    { "insert", KeyboardKey::KEY_INSERT },
    { "delete", KeyboardKey::KEY_DELETE },
    { "right", KeyboardKey::KEY_RIGHT },
    { "left", KeyboardKey::KEY_LEFT },
    { "down", KeyboardKey::KEY_DOWN },
    { "up", KeyboardKey::KEY_UP },
    { "page_up", KeyboardKey::KEY_PAGE_UP },
    { "page_down", KeyboardKey::KEY_PAGE_DOWN },
    { "home", KeyboardKey::KEY_HOME },
    { "end", KeyboardKey::KEY_END },
    { "caps_lock", KeyboardKey::KEY_CAPS_LOCK },
    { "scroll_lock", KeyboardKey::KEY_SCROLL_LOCK },
    { "num_lock", KeyboardKey::KEY_NUM_LOCK },
    { "print_screen", KeyboardKey::KEY_PRINT_SCREEN },
    { "pause", KeyboardKey::KEY_PAUSE },
    { "f1", KeyboardKey::KEY_F1 },
    { "f2", KeyboardKey::KEY_F2 },
    { "f3", KeyboardKey::KEY_F3 },
    { "f4", KeyboardKey::KEY_F4 },
    { "f5", KeyboardKey::KEY_F5 },
    { "f6", KeyboardKey::KEY_F6 },
    { "f7", KeyboardKey::KEY_F7 },
    { "f8", KeyboardKey::KEY_F8 },
    { "f9", KeyboardKey::KEY_F9 },
    { "f10", KeyboardKey::KEY_F10 },
    { "f11", KeyboardKey::KEY_F11 },
    { "f12", KeyboardKey::KEY_F12 },
    { "left_shift", KeyboardKey::KEY_LEFT_SHIFT },
    { "left_control", KeyboardKey::KEY_LEFT_CONTROL },
    { "left_alt", KeyboardKey::KEY_LEFT_ALT },
    { "left_super", KeyboardKey::KEY_LEFT_SUPER },
    { "right_shift", KeyboardKey::KEY_RIGHT_SHIFT },
    { "right_control", KeyboardKey::KEY_RIGHT_CONTROL },
    { "right_alt", KeyboardKey::KEY_RIGHT_ALT },
    { "right_super", KeyboardKey::KEY_RIGHT_SUPER },
    { "kb_menu", KeyboardKey::KEY_KB_MENU },
    { "kp_0", KeyboardKey::KEY_KP_0 },
    { "kp_1", KeyboardKey::KEY_KP_1 },
    { "kp_2", KeyboardKey::KEY_KP_2 },
    { "kp_3", KeyboardKey::KEY_KP_3 },
    { "kp_4", KeyboardKey::KEY_KP_4 },
    { "kp_5", KeyboardKey::KEY_KP_5 },
    { "kp_6", KeyboardKey::KEY_KP_6 },
    { "kp_7", KeyboardKey::KEY_KP_7 },
    { "kp_8", KeyboardKey::KEY_KP_8 },
    { "kp_9", KeyboardKey::KEY_KP_9 },
    { "kp_decimal", KeyboardKey::KEY_KP_DECIMAL },
    { "kp_divide", KeyboardKey::KEY_KP_DIVIDE },
    { "kp_multiply", KeyboardKey::KEY_KP_MULTIPLY },
    { "kp_subtract", KeyboardKey::KEY_KP_SUBTRACT },
    { "kp_add", KeyboardKey::KEY_KP_ADD },
    { "kp_enter", KeyboardKey::KEY_KP_ENTER },
    { "kp_equal", KeyboardKey::KEY_KP_EQUAL },
    { "back", KeyboardKey::KEY_BACK },
    { "menu", KeyboardKey::KEY_MENU },
    { "volume_up", KeyboardKey::KEY_VOLUME_UP },
    { "volume_down", KeyboardKey::KEY_VOLUME_DOWN },
};

int haspressed(lua_State* L) {
    int key = luaL_checkinteger(L, 1);
    bool pressed = IsKeyPressed(static_cast<KeyboardKey>(key));
    lua_pushboolean(L, pressed);
    return 1;
}

int isdown(lua_State* L) {
    int key = luaL_checkinteger(L, 1);
    bool down = IsKeyDown(static_cast<KeyboardKey>(key));
    lua_pushboolean(L, down);
    return 1;
}

} // namespace input

int adoreopen_input(lua_State* L)
{
    lua_createtable(L, 0, std::size(input::lib));
    
    for (auto& [name, func] : input::lib)
    {
        if (!name || !func)
            break;

        lua_pushcfunction(L, func, name);
        lua_setfield(L, -2, name);
    }

    lua_createtable(L, 0, std::size(input::kKeysMap));
    for (const auto& [name, key] : input::kKeysMap)
    {
        lua_pushinteger(L, static_cast<int>(key));
        lua_setfield(L, -2, name);
    }
    lua_setfield(L, -2, "keys");

    lua_setreadonly(L, -1, true);

    return 1;
}

