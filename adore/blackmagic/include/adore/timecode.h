#pragma once

#include "lua.h"
#include "lualib.h"

#include <string>

// open the library as a table on top of the stack
int adoreopen_timecode(lua_State* L);

constexpr int kTimecodeUserdataTag = 97;

namespace timecode
{

struct Timecode {
    int hours;
    int minutes;
    int seconds;
    int frames;
    bool dropFrame;
    int frameRate;

    std::string toString() const;

    Timecode addFrames(int frameCount) const;
    Timecode subtractFrames(int frameCount) const;
    Timecode addTimecode(const Timecode& other) const;
    Timecode subtractTimecode(const Timecode& other) const;
    int toTotalFrames() const;
    static Timecode fromTotalFrames(int totalFrames, int frameRate, bool dropFrame);
    static Timecode fromString(const std::string& str, int frameRate);
};

Timecode* checkTimecode(lua_State* L, int idx);

int parse(lua_State* L);
int from(lua_State* L);

static const luaL_Reg lib[] = {
    {"parse", parse},
    {"from", from},
    {nullptr, nullptr},
};

} // namespace timecode
