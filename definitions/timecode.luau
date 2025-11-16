
local timecode = {}

export type Timecode = {
    hours: number,
    minutes: number,
    seconds: number,
    frames: number,
    rate: number,
    total: number,
}

function timecode.parse(str: string, frameRate: number): Timecode
    error("Not implemented")
end

function timecode.from(totalFrames: number, frameRate: number, dropFrames: boolean?): string
    error("Not implemented")
end

return timecode
