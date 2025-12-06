
local window = require("@adore/window")
local graphics = require("@adore/graphics")
local color = require("@adore/color")

window.setstate(bit32.bor(
    window.states.topmost,
    window.states.transparent,
    window.states.always_run,
    window.states.mouse_passthrough,
    window.states.undecorated
))
local SCREEN_WIDTH = 100
local SCREEN_HEIGHT = 600

window.init(SCREEN_WIDTH, SCREEN_HEIGHT, "Christmas Candle Example")
window.setfps(30)

function window.update(dt: number)
    -- No update logic needed for this example
end

function window.draw()
    graphics.clearscreen(color.blank)
    
    graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color.white)
end

