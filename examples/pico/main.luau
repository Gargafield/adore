
local window = require("@adore/window")
local graphics = require("@adore/graphics")
local colors = require("@adore/colors")
local input = require("@adore/input")

local PICO_WIDTH = 128
local PICO_HEIGHT = 128
local SCALE = 4

local SCREEN_WIDTH = PICO_WIDTH * (SCALE + 1)
local SCREEN_HEIGHT = PICO_HEIGHT * SCALE

local PICO_OFFSET_X = (SCREEN_WIDTH - (PICO_WIDTH * SCALE)) / 2
local PICO_OFFSET_Y = (SCREEN_HEIGHT - (PICO_HEIGHT * SCALE)) / 2

window.init(SCREEN_WIDTH, SCREEN_HEIGHT, "PICO Example")
window.setfps(30)

local myRenderTexture = graphics.rendertexture.create(128, 128)

local playerPos = vector.create(64, 64)

function window.update(dt: number)
    if input.isdown(input.keys.w) or input.isdown(input.keys.up) then
        playerPos += vector.create(0, -100) * dt
    end
    if input.isdown(input.keys.s) or input.isdown(input.keys.down) then
        playerPos += vector.create(0, 100) * dt
    end
    if input.isdown(input.keys.a) or input.isdown(input.keys.left) then
        playerPos += vector.create(-100, 0) * dt
    end
    if input.isdown(input.keys.d) or input.isdown(input.keys.right) then
        playerPos += vector.create(100, 0) * dt
    end
end

function window.draw()
    graphics.clear(colors.black)

    graphics.rendertexture.start(myRenderTexture)
    graphics.clear(colors.white)
    graphics.circle("fill", playerPos.x, playerPos.y, 16, colors.skyblue)
    graphics.rendertexture.stop()

    graphics.texture.draw(myRenderTexture.texture, { 0, 0, PICO_WIDTH, -PICO_HEIGHT }, { PICO_OFFSET_X, PICO_OFFSET_Y, PICO_WIDTH * SCALE, PICO_HEIGHT * SCALE }, colors.white)
end

