
local gui = require("@adore/gui")
local hyperdeck = require("@adore/hyperdeck")

type Work = "play" | "pause"

return function (hyperdeck: hyperdeck.HyperDeck, on_work: (Work) -> ())
    local isplaying = hyperdeck.status == "play"

    return function (x: number, y: number, width: number, height: number)
        if isplaying then
            if gui.button(x, y, width, height, "Pause") then
                isplaying = false
                on_work("pause")
            end
        else
            if gui.button(x, y, width, height, "Play") then
                isplaying = true
                on_work("play")
            end
        end
    end
end