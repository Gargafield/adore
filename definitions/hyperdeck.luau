
local hyperdeck = {}

export type HyperDeckStatus = "preview" | "stopped" | "play" | "forward" | "rewind" | "jog" | "shuttle" | "record" | "unknown"
export type HyperDeckState = "disconnected" | "connecting" | "connected" | "error" | "unknown"
export type VideoFormatName = "720p50" | "720p5994" | "720p60" | "1080p23976" | "1080p24" | "1080p25" | "1080p2997" | "1080p30" | "1080p60" | "1080i50" | "1080i5994" | "1080i60"

export type Clip = {
    id: number,
    name: string,
    duration: string,
    timecode: string,
    format: VideoFormatName,
}

export type TimelineClip = {
    id: number,
    name: string,
    start: string,
    duration: string,
}

export type HyperDeck = {
    model: string,
    status: HyperDeckStatus,
    state: HyperDeckState,
    timecode: string,
    position: number?,
    ready: boolean,
    clip: number?,
    clips: { Clip },
    timeline: { TimelineClip },
    close: (self: HyperDeck) -> (),
    play: ((self: HyperDeck) -> ())
        & (self: HyperDeck, clip_id: number, settings: { loop: boolean, single: boolean }?) -> (),
    stop: (self: HyperDeck) -> (),
    goto: ((self: HyperDeck, mode: "timecode", value: string) -> ())
        & ((self: HyperDeck, mode: "position", value: number) -> ())
        & ((self: HyperDeck, mode: "clip", value: number) -> ()),
}

type VideoFormat = {
    name: string,
    width: number,
    height: number,
    framerate: number,
}

hyperdeck.formats = {} :: { [VideoFormatName]: VideoFormat }

function hyperdeck.connect(address: string): HyperDeck
    error("Not implemented")
end

return hyperdeck