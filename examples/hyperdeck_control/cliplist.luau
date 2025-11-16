
local hyperdeck = require("@adore/hyperdeck")
local gui = require("@adore/gui")
local graphics = require("@adore/graphics")
local color = require("@adore/color")

return function (hyperdeck: hyperdeck.HyperDeck)
    local scroll = -1
    return function (x: number, y: number, width: number, height: number)
        local clips = hyperdeck.timeline
        local playingclipid = hyperdeck.clip
        local textstyle = gui.getstyle("label", "text", "normal")
        scroll = gui.list(x, y, width, height, #clips, scroll, function (index: number, ix: number, iy: number, iw: number, ih: number)
            local clip = clips[index + 1]
            local thisplaying = (playingclipid ~= nil and clip.id == playingclipid)
            local text = string.format("%03d - %s (%s)", clip.id, clip.name, clip.duration)
            local timeposition = 0
            if thisplaying then
                gui.setstyle("label", "text", "normal", color.green)
            else
                gui.setstyle("label", "text", "normal", color.black)
            end
            gui.title(ix, iy, iw, 30, text)

            gui.sliderbar(ix, iy + 25, iw, 5, "start", "end", 50, 0, 100)
            graphics.rectangle("line", ix, iy, iw, 30, color.darkgray)

            return 30 -- item height
        end)
        gui.setstyle("label", "text", "normal", textstyle)
    end
end