
local window = {}

window.states = {} :: {
    vsync_hint: number,
    fullscreen_mode: number,
    resizable: number,
    undecorated: number,
    hidden: number,
    minimized: number,
    maximized: number,
    unfocused: number,
    topmost: number,
    always_run: number,
    transparent: number,
    highdpi: number,
    mouse_passthrough: number,
    borderless_windowed: number,
    msaa_4x_hint: number,
    interlaced_hint: number,
}

function window.setstate(state: number, enabled: boolean?)
    error("Not implemented")
end

function window.init(width: number, height: number, title: string)
    error("Not implemented")
end

function window.setfullscreen(fullscreen: boolean?)
    error("Not implemented")
end

function window.isfullscreen(): boolean
    error("Not implemented")
end

function window.setfps(fps: number)
    error("Not implemented")
end

function window.update(dt: number)
    error("Not implemented")
end

function window.draw()
    error("Not implemented")
end

function window.isfiledropped(): boolean
    error("Not implemented")
end

function window.getdroppedfiles(): {string}
    error("Not implemented")
end

function window.getmousescroll(): vector
    error("Not implemented")
end
function window.getmousepos(): vector
    error("Not implemented")
end

function window.getmonitorsize(): vector
    error("Not implemented")
end

window.setsize = (nil :: any) :: ((width: number, height: number) -> ())
    & ((size: vector) -> ())

window.setposition = (nil :: any) :: ((x: number, y: number) -> ())
    & ((position: vector) -> ())


return window
