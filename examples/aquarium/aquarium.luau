
local graphics = require("@adore/graphics")
local colors = require("@adore/colors")
local Fish = require("./fish")

local Aquarium = {}
Aquarium.__index = Aquarium

function Aquarium.new(width: number, height: number, x: number?, y: number?)
    local self = setmetatable({}, Aquarium)

    self.width = width
    self.height = height
    self.x = x or 0
    self.y = y or 0
    self.fish = {} :: { Fish.Fish }
    self.timer = 0

    return self
end

export type Aquarium = typeof(Aquarium.new(0, 0))

function Aquarium.display(self: Aquarium)
    graphics.rectangle("line", self.x, self.y, self.width, self.height, colors.black)
    
    local bottomY = self.y + self.height
    local sandHeight = self.height * 0.2
    local sandY = bottomY - sandHeight
    
    graphics.rectangle("fill", self.x + 1, sandY, self.width - 2, sandHeight - 1, colors.yellow)

    for _, _fish in self.fish do
        _fish:display(self.x, self.y)
    end
end

function Aquarium.spawn(self: Aquarium, amount: number)
    for i = 1, amount do
        table.insert(self.fish, Fish.new(math.random(0, self.width), math.random(0, self.height)))
    end
end

function Aquarium.update(self: Aquarium, dt: number)
    for _, _fish in self.fish do
        _fish:update(dt)
        _fish:checkBounds(self.width, self.height)
    end
end

return Aquarium

