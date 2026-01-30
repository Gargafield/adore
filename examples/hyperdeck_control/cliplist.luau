
local hyperdeck = require("@adore/hyperdeck")
local gui = require("@adore/gui")
local graphics = require("@adore/graphics")
local colors = require("@adore/colors")
local timecode = require("@adore/timecode")

return function (deck: hyperdeck.HyperDeck)
    local scroll = -1
    return function (x: number, y: number, width: number, height: number)
        local timelineClips = deck.timeline
        local diskClips = deck.clips
        local playingclipid = deck.clip

        local textstyle = gui.getstyle("label", "text", "normal")
        scroll = gui.list(x, y, width, height, #timelineClips, scroll, function (index: number, ix: number, iy: number, iw: number, ih: number)
            local timelineClip = timelineClips[index]
            local diskClip = diskClips[timelineClip.id]

            local thisplaying = (playingclipid ~= nil and timelineClip.id == playingclipid)
            local text = string.format("%03d - %s (%s)", timelineClip.id, diskClip.name, diskClip.duration)
            if thisplaying then
                gui.setstyle("label", "text", "normal", colors.green)
            else
                gui.setstyle("label", "text", "normal", colors.black)
            end
            gui.title(ix, iy, iw, 30, text)

            local format = hyperdeck.formats[diskClip.format]
            local startRange = timecode.parse(timelineClip.start, format.framerate).total + 1
            local endRange = timecode.parse(timelineClip.duration, format.framerate).total + startRange - 1
            local position = math.clamp(deck.position or 0, startRange, endRange)

            local scrub = gui.sliderbar(ix, iy + 25, iw, 5, timelineClip.start, timelineClip.duration, position, startRange, endRange)
            if scrub then
                scrub = math.max(math.round(scrub), 1)
                deck:goto("position", scrub)
            end
            
            graphics.rectangle("line", ix, iy, iw, 30, colors.darkgray)

            return 30 -- item height
        end)
        gui.setstyle("label", "text", "normal", textstyle)
    end
end