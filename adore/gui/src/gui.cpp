#include "adore/gui.h"

#include "adore/rect.h"
#include <memory>
#include <vector>
#include <iostream>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

namespace gui {

int button(lua_State* L)
{
    Rectangle rect;
    int end = rect::check_rect(L, 1, &rect);
    const char* text = luaL_checkstring(L, end);

    bool result = GuiButton(
        rect,
        text
    );
    lua_pushboolean(L, result);
    return 1;
}

int windowbox(lua_State* L)
{
    Rectangle rect;
    int end = rect::check_rect(L, 1, &rect);
    const char* text = luaL_checkstring(L, end);

    int result = GuiWindowBox(
        rect,
        text
    );
    lua_pushnumber(L, result);
    return 1;
}

int label(lua_State* L)
{
    Rectangle rect;
    int end = rect::check_rect(L, 1, &rect);
    const char* text = luaL_checkstring(L, end);

    GuiLabel(rect, text);
    return 0;
}

static int AdoreGuiComboBox(Rectangle bounds, std::vector<const char*> items, int *active)
{
    int result = 0;
    GuiState state = guiState;

    int temp = 0;
    if (active == NULL) active = &temp;

    bounds.width -= (GuiGetStyle(COMBOBOX, COMBO_BUTTON_WIDTH) + GuiGetStyle(COMBOBOX, COMBO_BUTTON_SPACING));

    Rectangle selector = { (float)bounds.x + bounds.width + GuiGetStyle(COMBOBOX, COMBO_BUTTON_SPACING),
                           (float)bounds.y, (float)GuiGetStyle(COMBOBOX, COMBO_BUTTON_WIDTH), (float)bounds.height };

    // Get substrings items from text (items pointers, lengths and count)
    int itemCount = items.size();

    if (*active < 0) *active = 0;
    else if (*active > (items.size() - 1)) *active = itemCount - 1;

    // Update control
    //--------------------------------------------------------------------
    if ((state != STATE_DISABLED) && !guiLocked && (itemCount > 1) && !guiControlExclusiveMode)
    {
        Vector2 mousePoint = GetMousePosition();

        if (CheckCollisionPointRec(mousePoint, bounds) ||
            CheckCollisionPointRec(mousePoint, selector))
        {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                *active += 1;
                if (*active >= itemCount) *active = 0;      // Cyclic combobox
            }

            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = STATE_PRESSED;
            else state = STATE_FOCUSED;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    // Draw combo box main
    GuiDrawRectangle(bounds, GuiGetStyle(COMBOBOX, BORDER_WIDTH), GetColor(GuiGetStyle(COMBOBOX, BORDER + (state*3))), GetColor(GuiGetStyle(COMBOBOX, BASE + (state*3))));
    GuiDrawText(items[*active], GetTextBounds(COMBOBOX, bounds), GuiGetStyle(COMBOBOX, TEXT_ALIGNMENT), GetColor(GuiGetStyle(COMBOBOX, TEXT + (state*3))));

    // Draw selector using a custom button
    // NOTE: BORDER_WIDTH and TEXT_ALIGNMENT forced values
    int tempBorderWidth = GuiGetStyle(BUTTON, BORDER_WIDTH);
    int tempTextAlign = GuiGetStyle(BUTTON, TEXT_ALIGNMENT);
    GuiSetStyle(BUTTON, BORDER_WIDTH, 1);
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);

    GuiButton(selector, TextFormat("%i/%i", *active + 1, itemCount));

    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, tempTextAlign);
    GuiSetStyle(BUTTON, BORDER_WIDTH, tempBorderWidth);
    //--------------------------------------------------------------------

    return result;
}

int combobox(lua_State* L) {
    Rectangle rect;
    int end = rect::check_rect(L, 1, &rect);
    luaL_checktype(L, end, LUA_TTABLE);
    std::vector<const char*> items;
    int itemCount = lua_objlen(L, end);
    for (int i = 1; i <= itemCount; i++) {
        lua_rawgeti(L, end, i);
        const char* item = luaL_checkstring(L, -1);
        items.push_back(item);
        lua_pop(L, 1);
    }

    if (lua_isstring(L, end + 1)) {
        const char* activeItem = luaL_checkstring(L, end + 1);
        int foundIndex = -1;
        for (size_t i = 0; i < items.size(); i++) {
            if (items[i] == activeItem) {
                foundIndex = static_cast<int>(i);
                break;
            }
        }
        if (foundIndex == -1) {
            foundIndex = 0; // Default to first item if not found
        }
        AdoreGuiComboBox(
            rect,
            items,
            &foundIndex
        );
        lua_rawgeti(L, end, foundIndex + 1);
        return 1;
    } else if (lua_isnumber(L, end + 1)) {
        int activeIndex = static_cast<int>(luaL_checknumber(L, end + 1));
        AdoreGuiComboBox(
            rect,
            items,
            &activeIndex
        );
        lua_pushnumber(L, activeIndex);
        return 1;
    } else {
        AdoreGuiComboBox(
            rect,
            items,
            nullptr
        );
        return 0;
    }
}

int textbox(lua_State* L)
{
    Rectangle rect;
    int end = rect::check_rect(L, 1, &rect);
    size_t buflen;
    void* buffer = luaL_checkbuffer(L, end, &buflen);
    bool editMode = luaL_checkboolean(L, end + 1);

    int result = GuiTextBox(
        rect,
        static_cast<char*>(buffer),
        buflen - 1,
        editMode
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

int enable(lua_State* L)
{
    GuiEnable();
    return 0;
}

int disable(lua_State* L)
{
    GuiDisable();
    return 0;
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

