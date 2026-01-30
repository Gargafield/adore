
local graphics = require("@adore/graphics")
local color = require("@adore/color")

local Fish = {}
Fish.__index = Fish

function Fish.new(x: number?, y: number?, _color: color.Color?)
    local self = setmetatable({}, Fish)

    self.position = vector.create(x or 0, y or 0)
    self.velocity = vector.create(0, 0)
    self.color = _color
        or (vector.create :: any)(
            math.random(0, 255),
            math.random(0, 255),
            math.random(0, 255),
            255
        )

    return self
end

export type Fish = typeof(Fish.new(0, 0))

function Fish.display(self: Fish, offsetX: number, offsetY: number)
    graphics.circle("fill", self.position.x + offsetX, self.position.y + offsetY, 3, self.color)
end

function Fish.update(self: Fish, dt: number)
    self.velocity += vector.create(
        math.random(-20, 20),
        math.random(-10, 10)
    ) * dt * 10
    self.velocity = vector.lerp(vector.zero, self.velocity, math.min(0.99 * dt * 60, 0.99))
    self.position = self.position + self.velocity * dt
end

function Fish.checkBounds(self: Fish, width: number, height: number)

    self.velocity = vector.create(
        if self.position.x < 0 or self.position.x > width
            then -self.velocity.x
            else self.velocity.x,
        if self.position.y < 0 or self.position.y > height
            then -self.velocity.y
            else self.velocity.y
    )
    
    self.position = vector.create(
        math.clamp(self.position.x, 0, width),
        math.clamp(self.position.y, 0, height)
    )
end

return Fish

