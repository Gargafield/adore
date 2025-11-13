
local gui = {}

gui.button = (nil :: any) ::
    ((x: number, y: number, width: number, height: number, text: string) -> boolean)
    & ((box: { number }, text: string) -> boolean)

gui.windowbox = (nil :: any) ::
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


function gui.enable()
    error("Not implemented")
end

function gui.disable()
    error("Not implemented")
end

return gui