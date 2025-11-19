
local graphics = require("@adore/graphics")
local input = require("@adore/input")
local window = require("@adore/window")
local hyperdeck = require("@adore/hyperdeck")
local atem = require("@adore/atem")
local gui = require("@adore/gui")

local connector = require("./connector")

local SCREEN_WIDTH = 1440
local SCREEN_HEIGHT = 810

window.init(SCREEN_WIDTH, SCREEN_HEIGHT, "AdoreCue")
window.setfps(30)

local devicelist = require("./devicelist")()

function window.update(dt)

end

function window.draw()
    devicelist(SCREEN_WIDTH - (SCREEN_WIDTH / 4), 0, (SCREEN_WIDTH / 4), SCREEN_HEIGHT)
end


