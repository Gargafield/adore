local window = require("@adore/window")
local graphics = require("@adore/graphics")
local color = require("@adore/color")
local gui = require("@adore/gui")

local SCREEN_WIDTH = 800
local SCREEN_HEIGHT = 450

window.init(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello World!")
window.setfps(60)

function window.update(dt)
end

function window.draw()
    graphics.clearscreen(color.raywhite)

    graphics.print("Welcome to Adore GUI!", 10, 10, 10, color.darkgray)
    if gui.button(50, 0, 200, 20, "Click Me!") then
        print("Button Clicked!")
    end
end
