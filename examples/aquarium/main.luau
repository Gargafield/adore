
local window = require("@adore/window")
local graphics = require("@adore/graphics")
local colors = require("@adore/colors")
local Aquarium = require("./aquarium")
local Fish = require("./fish")

local SCREEN_WIDTH = 800
local SCREEN_HEIGHT = 450

window.init(SCREEN_WIDTH, SCREEN_HEIGHT, "Aquarium Example")
window.setfps(30)

local myAquarium = Aquarium.new(600, 300, 0, 0)
myAquarium:spawn(100)

local myRenderTexture = graphics.rendertexture.create(600, 300)

function window.update(dt: number)
    myAquarium:update(dt)
end

function window.draw()
    graphics.clear(colors.white)

    graphics.rendertexture.start(myRenderTexture)
    graphics.clear(colors.skyblue)
    myAquarium:display()
    graphics.rendertexture.stop()

    graphics.texture.drawflipped(myRenderTexture.texture, 100, 75, "y")
end

