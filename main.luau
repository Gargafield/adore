local window = require("@adore/window")
local graphics = require("@adore/graphics")

local SCREEN_WIDTH = 800
local SCREEN_HEIGHT = 450

window.init(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello World!")
window.setfps(60)

local deltaCircle = vector.create(0, SCREEN_HEIGHT / 3)
local frameCircle = vector.create(0, (SCREEN_HEIGHT / 3) * 2)

local speed = 10
local circleRadius = 32

function window.update(dt)
    deltaCircle += vector.create(speed * dt * 60, 0)
    if deltaCircle.x - circleRadius > SCREEN_WIDTH then
        deltaCircle = vector.create(0, deltaCircle.y)
    end

    frameCircle += vector.create(speed, 0)
    if frameCircle.x - circleRadius > SCREEN_WIDTH then
        frameCircle = vector.create(0, frameCircle.y)
    end

    if math.random(0, 20) == 0 then
        print("LAG!!!")
        for i = 1, 1000000 do
            local _ = math.sqrt(i)
        end
    end
end

function window.draw()
    graphics.setcolor(graphics.colors.raywhite)
    graphics.clearscreen()
    graphics.setcolor(graphics.colors.red)
    graphics.circle("fill", deltaCircle.x, deltaCircle.y, circleRadius)
    graphics.setcolor(graphics.colors.blue)
    graphics.circle("fill", frameCircle.x, frameCircle.y, circleRadius)

    graphics.setfontsize(20)
    graphics.setcolor(graphics.colors.darkgray)
    local text = "FPS: " .. tostring(window.getfps())
    graphics.print(text, 10, 10)
end

print("Hello World!")
