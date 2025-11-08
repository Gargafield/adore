local window = require("@adore/window")
local graphics = require("@adore/graphics")

window.init(800, 420, "Hello World!")

function window.draw(dt)
    graphics.rectangle(0, 0, 100, 100)
end

print("Hello World!")
