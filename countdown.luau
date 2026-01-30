--!strict

local graphics = require("@adore/graphics")
local task = require("@lute/task")
local window = require("@adore/window")
local time = require("@lute/time")
local colors = require("@adore/colors")
local input = require("@adore/input")

local SCREEN_WIDTH = 1920
local SCREEN_HEIGHT = 1080

window.init(SCREEN_WIDTH, SCREEN_HEIGHT, "Countdown")
window.setfps(60)

local font = graphics.font.load("build/Rocket Rinder.otf", 200)
graphics.texture.setfilter(font.texture, graphics.texture.filter.bilinear)

local startTime = time.now()
local countdown = 60 * 60; -- 60 minutes
local timer = countdown
local paused = true

local green = vector.create(0, 255, 0)

function window.update(dt)
    if not paused then
        timer = math.max(countdown - (time.now() - startTime):toseconds(), 0)
    end

    if input.ispressed(input.keys["f"]) then
        window.setfullscreen(not window.isfullscreen())
    end
    if input.ispressed(input.keys["space"]) then
        paused = not paused
        if paused then
            countdown = timer
        else
            startTime = time.now()
        end
    end

    countdown += window.getmousescroll().y * 5
end

function window.draw()
    graphics.clearscreen(green)
    
    local minutes = math.floor(timer / 60)
    local seconds = math.floor(timer % 60)
    local text = string.format("%02d:%02d", minutes, seconds)
    local fontsize = 200
    local size = graphics.font.measure(font, fontsize, text, 10)
    
    graphics.font.draw(font, SCREEN_WIDTH / 2 - size.x / 2, SCREEN_HEIGHT / 2 - size.y / 2, fontsize, text, color.black, 10)
end

