
local bufutil = {}

function bufutil.readcstring(buf: buffer, offset: number): (string)
    local endpos = 1
    while endpos < buffer.len(buf) and buffer.readu8(buf, endpos) ~= 0 do
        endpos += 1
    end
    return buffer.readstring(buf, 0, endpos)
end


return bufutil