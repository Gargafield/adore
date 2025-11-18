
local window = {}

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

return window
