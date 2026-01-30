
local window = require("@adore/window")
local graphics = require("@adore/graphics")
local colors = require("@adore/colors")
local Aquarium = require("./aquarium")
local Fish = require("./fish")

local SCREEN_WIDTH = 800
local SCREEN_HEIGHT = 450

window.init(SCREEN_WIDTH, SCREEN_HEIGHT, "Aquarium Example")
window.setfps(30)

local myAquarium = Aquarium.new(600, 300, 100, 75)
myAquarium:spawn(100)

function window.update(dt: number)
    myAquarium:update(dt)
end

function window.draw()
    graphics.clearscreen(colors.white)

    myAquarium:display()
end

