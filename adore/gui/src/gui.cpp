#include "adore/gui.h"

#include "adore/rect.h"
#include <memory>
#include <vector>
#include <iostream>
#include <string>

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

int title(lua_State* L)
{
    Rectangle rect;
    int end = rect::check_rect(L, 1, &rect);
    const char* text = luaL_checkstring(L, end);
    int textSize = GuiGetStyle(DEFAULT, TEXT_SIZE);
    int verticalAlign = GuiGetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL);
    GuiSetStyle(DEFAULT, TEXT_SIZE, rect.height - 4);
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_CENTER);
    GuiLabel(rect, text);
    GuiSetStyle(DEFAULT, TEXT_SIZE, textSize);
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, verticalAlign);
    return 0;
}

int sliderbar(lua_State* L)
{
    Rectangle rect;
    int end = rect::check_rect(L, 1, &rect);
    const char* textLeft = luaL_optlstring(L, end, NULL, NULL);
    const char* textRight = luaL_optlstring(L, end + 1, NULL, NULL);
    float value = static_cast<float>(luaL_checknumber(L, end + 2));
    float minValue = static_cast<float>(luaL_checknumber(L, end + 3));
    float maxValue = static_cast<float>(luaL_checknumber(L, end + 4));

    int result = GuiSliderBar(
        rect,
        textLeft,
        textRight,
        &value,
        minValue,
        maxValue
    );

    if (result) {
        lua_pushnumber(L, value);
    } else {
        lua_pushnil(L);
    }
    
    return 1;
}

int panel(lua_State* L)
{
    Rectangle rect;
    int end = rect::check_rect(L, 1, &rect);
    const char* text = nullptr;
    if (lua_isstring(L, end)) {
        text = luaL_checkstring(L, end);
    }
    GuiPanel(
        rect,
        text
    );
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

int list(lua_State* L) {
    Rectangle bounds;
    int end = rect::check_rect(L, 1, &bounds);
    int count = luaL_checknumber(L, end);
    int scrollIndex = luaL_optnumber(L, end + 1, -1);
    // callback
    luaL_checktype(L, end + 2, LUA_TFUNCTION);

    int result = 0;
    GuiState state = guiState;

    // Check if we need a scroll bar
    bool useScrollBar = scrollIndex >= 0;

    // Define base item rectangle [0]
    Rectangle itemBounds = { 0 };
    itemBounds.x = bounds.x + GuiGetStyle(LISTVIEW, LIST_ITEMS_SPACING);
    itemBounds.y = bounds.y + GuiGetStyle(LISTVIEW, LIST_ITEMS_SPACING) + GuiGetStyle(DEFAULT, BORDER_WIDTH);
    itemBounds.width = bounds.width - 2*GuiGetStyle(LISTVIEW, LIST_ITEMS_SPACING) - GuiGetStyle(DEFAULT, BORDER_WIDTH);
    itemBounds.height = (float)(bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - 2*GuiGetStyle(LISTVIEW, LIST_ITEMS_SPACING));
    if (useScrollBar) itemBounds.width -= GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH);

    // Get items on the list
    int startIndex = (scrollIndex <= 0 ) ? 0 : scrollIndex;
    if (startIndex > (count - 1)) startIndex = 0;

    // Update control
    //--------------------------------------------------------------------
    if ((state != STATE_DISABLED) && !guiLocked && !guiControlExclusiveMode)
    {
        Vector2 mousePoint = GetMousePosition();

        // Check mouse inside list view
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            state = STATE_FOCUSED;

            if (useScrollBar)
            {
                int wheelMove = (int)GetMouseWheelMove();
                startIndex -= wheelMove;

                if (startIndex < 0) startIndex = 0;
                else if (startIndex > (count - 1)) startIndex = 0;
            }
        }

        // Reset item rectangle y to [0]
        itemBounds.y = bounds.y + GuiGetStyle(LISTVIEW, LIST_ITEMS_SPACING) + GuiGetStyle(DEFAULT, BORDER_WIDTH);
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawRectangle(bounds, GuiGetStyle(LISTVIEW, BORDER_WIDTH), GetColor(GuiGetStyle(LISTVIEW, BORDER + state*3)), GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));     // Draw background

    // Draw visible items
    int i;
    for (i = startIndex; i < count; i++) {
        lua_pushvalue(L, end + 2); // callback function
        lua_pushnumber(L, i + 1); // item index (1-based)
        lua_pushnumber(L, itemBounds.x);
        lua_pushnumber(L, itemBounds.y);
        lua_pushnumber(L, itemBounds.width);
        lua_pushnumber(L, itemBounds.height);
        BeginScissorMode(
            (int)bounds.x + GuiGetStyle(LISTVIEW, BORDER_WIDTH),
            (int)bounds.y + GuiGetStyle(LISTVIEW, BORDER_WIDTH),
            (int)(bounds.width - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH)),
            (int)(bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH))
        );
        lua_call(L, 5, 1);
        EndScissorMode();
        int height = (int)luaL_checknumber(L, -1);
        lua_pop(L, 1);
        if (height <= 0) height = 0;
        float usedHeight = (height + GuiGetStyle(LISTVIEW, LIST_ITEMS_SPACING));
        itemBounds.y += usedHeight;
        itemBounds.height -= usedHeight;
        if (itemBounds.height <= 0) {
            break;
        }
    }

    int endIndex = i;
    int visibleItems = endIndex - startIndex;

    if (useScrollBar)
    {
        Rectangle scrollBarBounds = {
            bounds.x + bounds.width - GuiGetStyle(LISTVIEW, BORDER_WIDTH) - GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH),
            bounds.y + GuiGetStyle(LISTVIEW, BORDER_WIDTH), (float)GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH),
            bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH)
        };

        // Calculate percentage of visible items and apply same percentage to scrollbar
        float percentVisible = (float)(endIndex - startIndex)/count;
        float sliderSize = bounds.height*percentVisible;

        int prevSliderSize = GuiGetStyle(SCROLLBAR, SCROLL_SLIDER_SIZE);   // Save default slider size
        int prevScrollSpeed = GuiGetStyle(SCROLLBAR, SCROLL_SPEED); // Save default scroll speed
        GuiSetStyle(SCROLLBAR, SCROLL_SLIDER_SIZE, (int)sliderSize);            // Change slider size
        GuiSetStyle(SCROLLBAR, SCROLL_SPEED, count - visibleItems); // Change scroll speed

        startIndex = GuiScrollBar(scrollBarBounds, startIndex, 0, count - visibleItems);

        GuiSetStyle(SCROLLBAR, SCROLL_SPEED, prevScrollSpeed); // Reset scroll speed to default
        GuiSetStyle(SCROLLBAR, SCROLL_SLIDER_SIZE, prevSliderSize); // Reset slider size to default
    }
    //--------------------------------------------------------------------

    if (startIndex <= 0 && endIndex == count) {
        // All items are visible, no scrolling needed
        lua_pushnumber(L, -1);
    } else {
        lua_pushnumber(L, startIndex);
    }

    return 1;
}

int valuebox(lua_State* L)
{
    Rectangle rect;
    int end = rect::check_rect(L, 1, &rect);
    int value = static_cast<int>(luaL_checknumber(L, end));
    int minValue = static_cast<int>(luaL_checknumber(L, end + 1));
    int maxValue = static_cast<int>(luaL_checknumber(L, end + 2));
    const char* text = luaL_optlstring(L, end + 3, NULL, NULL);
    bool editMode = luaL_optboolean(L, end + 4, true);

    int result = GuiValueBox(
        rect,
        text,
        &value,
        minValue,
        maxValue,
        editMode
    );

    lua_pushnumber(L, value);

    return 1;
}

int spinner(lua_State* L)
{
    Rectangle rect;
    int end = rect::check_rect(L, 1, &rect);
    int value = static_cast<int>(luaL_checknumber(L, end));
    int minValue = static_cast<int>(luaL_checknumber(L, end + 1));
    int maxValue = static_cast<int>(luaL_checknumber(L, end + 2));
    const char* text = luaL_optlstring(L, end + 3, NULL, NULL);
    bool editMode = luaL_optboolean(L, end + 4, true);

    int result = GuiSpinner(
        rect,
        text,
        &value,
        minValue,
        maxValue,
        editMode
    );

    lua_pushnumber(L, value);

    return 1;
}

static const std::pair<const char*, GuiControl> guiControls[] = {
    { "default", DEFAULT },
    { "label", LABEL },
    { "button", BUTTON },
    { "toggle", TOGGLE },
    { "slider", SLIDER },
    { "progressbar", PROGRESSBAR },
    { "checkbox", CHECKBOX },
    { "combobox", COMBOBOX },
    { "dropdownbox", DROPDOWNBOX },
    { "textbox", TEXTBOX },
    { "valuebox", VALUEBOX },
    { "control11", CONTROL11 },
    { "listview", LISTVIEW },
    { "colorpicker", COLORPICKER },
    { "scrollbar", SCROLLBAR },
    { "statusbar", STATUSBAR }
};

static const std::pair<const char*, int> guiProperties[] = {
    { "border_width", BORDER_WIDTH },
    { "text_alignment", TEXT_ALIGNMENT },
    { "text", TEXT },
    { "base", BASE },
    { "border", BORDER },
    { "scrollbar_width", SCROLLBAR_WIDTH },
    { "scroll_slider_size", SCROLL_SLIDER_SIZE },
    { "scroll_slider_padding", SCROLL_SLIDER_PADDING },
    { "scroll_padding", SCROLL_PADDING },
    { "scroll_speed", SCROLL_SPEED },
    { "list_items_spacing", LIST_ITEMS_SPACING },
    { "text_size", TEXT_SIZE },
    { "text_spacing", TEXT_SPACING },
    { "line_color", LINE_COLOR },
    { "background_color", BACKGROUND_COLOR },
    { "text_line_spacing", TEXT_LINE_SPACING },
    { "text_alignment_vertical", TEXT_ALIGNMENT_VERTICAL },
    { "text_wrap_mode", TEXT_WRAP_MODE }
};

static const std::pair<const char*, int> guiStates[] = {
    { "none", 0 },
    { "normal", STATE_NORMAL },
    { "focused", STATE_FOCUSED },
    { "pressed", STATE_PRESSED },
    { "disabled", STATE_DISABLED }
};

int getstyle(lua_State* L)
{
    const char* controlName = luaL_checkstring(L, 1);
    const char* propertyName = luaL_checkstring(L, 2);
    const char* stateName = luaL_checkstring(L, 3);

    for (const auto& controlPair : guiControls) {
        if (strcmp(controlName, controlPair.first) == 0) {
            for (const auto& propertyPair : guiProperties) {
                if (strcmp(propertyName, propertyPair.first) == 0) {
                    for (const auto& statePair : guiStates) {
                        if (strcmp(stateName, statePair.first) == 0) {
                            int property = propertyPair.second;
                            if (property < RAYGUI_MAX_PROPS_BASE) {
                                property += statePair.second * 3;
                            }
                            int value = GuiGetStyle(controlPair.second, property);
                            lua_pushnumber(L, value);
                            return 1;
                        }
                    }
                    luaL_error(L, "Unknown GUI state: %s", stateName);
                }
            }
            luaL_error(L, "Unknown GUI property: %s", propertyName);
        }
    }

    luaL_error(L, "Unknown GUI control: %s", controlName);
    return 1;
}

int setstyle(lua_State* L)
{
    const char* controlName = luaL_checkstring(L, 1);
    const char* propertyName = luaL_checkstring(L, 2);
    const char* stateName = luaL_checkstring(L, 3);
    int value;
    if (lua_isnumber(L, 4)) {
        value = static_cast<int>(luaL_checknumber(L, 4));
    } else if (lua_isvector(L, 4)) {
        const float* vec = luaL_checkvector(L, 4);
        value = ColorToInt(Color {
            static_cast<unsigned char>(vec[0]),
            static_cast<unsigned char>(vec[1]),
            static_cast<unsigned char>(vec[2]),
            255
        });
    } else {
        luaL_error(L, "Expected number or color for style value");
    }

    for (const auto& controlPair : guiControls) {
        if (strcmp(controlName, controlPair.first) == 0) {
            for (const auto& propertyPair : guiProperties) {
                if (strcmp(propertyName, propertyPair.first) == 0) {
                    for (const auto& statePair : guiStates) {
                        if (strcmp(stateName, statePair.first) == 0) {
                            int property = propertyPair.second;
                            if (property < RAYGUI_MAX_PROPS_BASE) {
                                property += statePair.second * 3;
                            }
                            GuiSetStyle(controlPair.second, property, value);
                            return 0;
                        }
                    }
                    luaL_error(L, "Unknown GUI state: %s", stateName);
                }
            }
            luaL_error(L, "Unknown GUI property: %s", propertyName);
        }
    }

    luaL_error(L, "Unknown GUI control: %s", controlName);
    return 0;
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

