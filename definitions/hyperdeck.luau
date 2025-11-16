
local hyperdeck = {}

export type HyperDeckStatus = "preview" | "stopped" | "play" | "forward" | "rewind" | "jog" | "shuttle" | "record" | "unknown"
export type HyperDeckState = "disconnected" | "connecting" | "connected" | "error" | "unknown"

export type Clip = {
    id: number,
    name: string,
    duration: string,
    timecode: string,
    format: string,
}

export type HyperDeck = {
    model: string,
    status: HyperDeckStatus,
    state: HyperDeckState,
    ready: boolean,
    clip: number?,
    clips: { Clip },
    close: (self: HyperDeck) -> (),
    play: (self: HyperDeck) -> (),
    stop: (self: HyperDeck) -> (),
}

function hyperdeck.connect(address: string): HyperDeck
    error("Not implemented")
end

return hyperdeck