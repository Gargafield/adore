
local graphics = require("@adore/graphics")
local task = require("@lute/task")
local window = require("@adore/window")
local time = require("@lute/time")
local color = require("@adore/color")
local input = require("@adore/input")

local SCREEN_WIDTH = 1920
local SCREEN_HEIGHT = 1080

window.init(SCREEN_WIDTH, SCREEN_HEIGHT, "Countdown")
window.setfps(60)

local startTime = time.now()
local countdown = 60 * 60 * 60; -- 60 minutes
local timer = countdown

local green = vector.create(0, 255, 0)

function window.update(dt)
    timer = math.max(countdown - time.since(startTime), 0)

    if input.ispressed(input.keys["f"]) then
        window.setfullscreen(not window.isfullscreen())
    end
end

function window.draw()
    graphics.clearscreen(green)
    
    local minutes = math.floor(timer / (60 * 60))
    local seconds = math.floor(timer % 60)
    local text = string.format("%02d:%02d", minutes, seconds)
    local fontsize = 100
    
    graphics.print(text, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, fontsize, color.black)
end

