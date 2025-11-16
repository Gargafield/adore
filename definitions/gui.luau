
local gui = {}

gui.button = (nil :: any) ::
    ((x: number, y: number, width: number, height: number, text: string) -> boolean)
    & ((box: { number }, text: string) -> boolean)

gui.windowbox = (nil :: any) ::
    ((x: number, y: number, width: number, height: number, text: string) -> boolean)
    & ((box: { number }, text: string) -> boolean)

gui.title = (nil :: any) ::
    ((x: number, y: number, width: number, height: number, text: string) -> boolean)
    & ((box: { number }, text: string) -> boolean)

gui.label = (nil :: any) ::
    ((x: number, y: number, width: number, height: number, text: string) -> boolean)
    & ((box: { number }, text: string) -> boolean)

gui.combobox = (nil :: any) ::
    ((x: number, y: number, width: number, height: number, items: { string }, active: number) -> number)
    & ((box: { number }, items: { string }, active: number) -> number)
    & ((x: number, y: number, width: number, height: number, items: { string }, active: string) -> string)
    & ((box: { number }, items: { string }, active: string) -> string)

gui.textbox = (nil :: any) ::
    ((x: number, y: number, width: number, height: number, buf: buffer, editmode: boolean) -> boolean)
    & ((box: { number }, buf: buffer, editmode: boolean) -> boolean)

gui.list = (nil :: any) ::
    ((x: number, y: number, width: number, height: number, count: number, scroll: number, callback: (index: number, x: number, y: number, width: number, height: number) -> number) -> number)
    & ((box: { number }, count: number, scroll: number, callback: (index: number, x: number, y: number, width: number, height: number) -> number) -> number)

type GuiControls = "default" | "label" | "button" | "toggle" | "slider" | "progressbar" | "checkbox" | "combobox" | "dropdownbox" | "textbox" | "valuebox" | "control11" | "listview" | "colorpicker" | "scrollbar" | "statusbar" 
type GuiStates = "none" | "normal" | "focused" | "pressed" | "disabled"
type GuiProperties = "border_width" | "text_alignment" | "text" | "base" | "border" | "scrollbar_width" | "scroll_slider_size" | "scroll_slider_padding" | "scroll_padding" | "scroll_speed" | "list_items_spacing" | "text_size" | "text_spacing" | "line_color" | "background_color" | "text_line_spacing" | "text_alignment_vertical" | "text_wrap_mode"

gui.setstyle = (nil :: any) ::
    ((control: GuiControls, property: GuiProperties, state: GuiStates, value: number | vector) -> ())

gui.getstyle = (nil :: any) ::
    ((control: GuiControls, property: GuiProperties, state: GuiStates) -> number | vector)

function gui.enable()
    error("Not implemented")
end

function gui.disable()
    error("Not implemented")
end

return gui