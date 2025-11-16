--!strict

local hyperdeck = require("@adore/hyperdeck")
local window = require("@adore/window")
local graphics = require("@adore/graphics")
local color = require("@adore/color")
local gui = require("@adore/gui")
local task = require("@lute/task")
local timecode = require("@adore/timecode")

local playbutton = require("./playbutton")
local cliplist = require("./cliplist")

window.init(800, 600, "HyperDeck Control Example")
window.setfps(30)

local deck = hyperdeck.connect("192.168.0.95");

local myplaybutton = playbutton(deck, function (work: string)
    if work == "play" then
        deck:play()
    elseif work == "pause" then
        deck:stop()
    end
end)

local mycliplist = cliplist(deck)

function window.update(dt)

end

function window.draw()
    graphics.clearscreen(color.raywhite)
    myplaybutton(0, 0, 100, 50)
    gui.label(110, 00, 200, 10, "Deck Status: " .. deck.status)
    gui.label(110, 20, 200, 10, "Timecode: " .. tostring(deck.timecode))
    if deck.clip then
        local playingclip = deck.clips[deck.clip]
        local format = hyperdeck.formats[playingclip.format]
        if format then
            gui.label(110, 40, 300, 10, string.format("Format: %s (%dx%d @ %.2f fps)", format.name, format.width, format.height, format.framerate))
        end
    end
    mycliplist(0, 80, 400, 540)
end
