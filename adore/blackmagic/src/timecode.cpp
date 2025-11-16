#include "adore/timecode.h"

namespace timecode {

Timecode Timecode::addFrames(int frameCount) const {
    int totalFrames = this->toTotalFrames() + frameCount;
    return fromTotalFrames(totalFrames, this->frameRate, this->dropFrame);
}

Timecode Timecode::subtractFrames(int frameCount) const {
    int totalFrames = this->toTotalFrames() - frameCount;
    return fromTotalFrames(totalFrames, this->frameRate, this->dropFrame);
}

Timecode Timecode::addTimecode(const Timecode& other) const {
    int totalFrames = this->toTotalFrames() + other.toTotalFrames();
    return fromTotalFrames(totalFrames, this->frameRate, this->dropFrame);
}

Timecode Timecode::subtractTimecode(const Timecode& other) const {
    int totalFrames = this->toTotalFrames() - other.toTotalFrames();
    return fromTotalFrames(totalFrames, this->frameRate, this->dropFrame);
}

Timecode Timecode::fromString(const std::string& str, int frameRate) {
    // Simple parser for "HH:MM:SS:FF" or "HH:MM:SS;FF"
    Timecode tc;
    tc.frameRate = frameRate;
    tc.dropFrame = (str.find(';') != std::string::npos);

    sscanf(str.c_str(), "%d:%d:%d:%d", &tc.hours, &tc.minutes, &tc.seconds, &tc.frames);
    return tc;
}

Timecode Timecode::fromTotalFrames(int totalFrames, int frameRate, bool dropFrame) {
    Timecode tc;
    tc.frameRate = frameRate;
    tc.dropFrame = dropFrame;

    int framesPerHour = frameRate * 3600;
    int framesPerMinute = frameRate * 60;
    int framesPerSecond = frameRate;

    tc.hours = totalFrames / framesPerHour;
    totalFrames %= framesPerHour;

    tc.minutes = totalFrames / framesPerMinute;
    totalFrames %= framesPerMinute;

    tc.seconds = totalFrames / framesPerSecond;
    totalFrames %= framesPerSecond;

    tc.frames = totalFrames;

    return tc;
}

int Timecode::toTotalFrames() const {
    int totalFrames = 0;
    totalFrames += this->hours * this->frameRate * 3600;
    totalFrames += this->minutes * this->frameRate * 60;
    totalFrames += this->seconds * this->frameRate;
    totalFrames += this->frames;
    return totalFrames;
}

std::string Timecode::toString() const {
    char buffer[12];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d%c%02d",
             hours, minutes, seconds,
             dropFrame ? ';' : ':',
             frames);
    return std::string(buffer);
}


Timecode* checkTimecode(lua_State* L, int idx) {
    return static_cast<Timecode*>(luaL_checkudata(L, idx, "Timecode"));
}

int createTimecode(lua_State* L, const Timecode& tc) {
    Timecode* userdata = static_cast<Timecode*>(lua_newuserdatatagged(L, sizeof(Timecode), kTimecodeUserdataTag));
    *userdata = tc;

    luaL_getmetatable(L, "Timecode");
    lua_setmetatable(L, -2);

    return 1;
}

int parse(lua_State* L) {
    const char* str = luaL_checkstring(L, 1);
    int frameRate = luaL_checkinteger(L, 2);

    Timecode tc = Timecode::fromString(str, frameRate);
    return createTimecode(L, tc);
}

int from(lua_State* L) {
    int totalFrames = luaL_checkinteger(L, 1);
    int frameRate = luaL_checkinteger(L, 2);
    bool dropFrame = luaL_optboolean(L, 3, false);

    Timecode tc = Timecode::fromTotalFrames(totalFrames, frameRate, dropFrame);
    return createTimecode(L, tc);
}

int tostring(lua_State* L) {
    Timecode* tc = checkTimecode(L, 1);
    std::string str = tc->toString();
    lua_pushstring(L, str.c_str());
    return 1;
}

int add(lua_State* L) {
    Timecode* left = checkTimecode(L, 1);

    if (lua_isnumber(L, 2)) {
        int frameCount = luaL_checkinteger(L, 2);
        Timecode result = left->addFrames(frameCount);
        return createTimecode(L, result);
    } else {
        Timecode* right = checkTimecode(L, 2);
        Timecode result = left->addTimecode(*right);
        return createTimecode(L, result);
    }
}

int subtract(lua_State* L) {
    Timecode* left = checkTimecode(L, 1);

    if (lua_isnumber(L, 2)) {
        int frameCount = luaL_checkinteger(L, 2);
        Timecode result = left->subtractFrames(frameCount);
        return createTimecode(L, result);
    } else {
        Timecode* right = checkTimecode(L, 2);
        Timecode result = left->subtractTimecode(*right);
        return createTimecode(L, result);
    }
}

int index(lua_State* L) {
    const char* key = luaL_checkstring(L, 2);
    Timecode* tc = checkTimecode(L, 1);

    if (strcmp(key, "hours") == 0) {
        lua_pushinteger(L, tc->hours);
        return 1;
    } else if (strcmp(key, "minutes") == 0) {
        lua_pushinteger(L, tc->minutes);
        return 1;
    } else if (strcmp(key, "seconds") == 0) {
        lua_pushinteger(L, tc->seconds);
        return 1;
    } else if (strcmp(key, "frames") == 0) {
        lua_pushinteger(L, tc->frames);
        return 1;
    } else if (strcmp(key, "drop") == 0) {
        lua_pushboolean(L, tc->dropFrame);
        return 1;
    } else if (strcmp(key, "rate") == 0) {
        lua_pushinteger(L, tc->frameRate);
        return 1;
    } else if (strcmp(key, "total") == 0) {
        int totalFrames = tc->toTotalFrames();
        lua_pushinteger(L, totalFrames);
        return 1;
    }

    lua_pushnil(L);
    return 1;
}

} // namespace timecode

static int adoreregister_timecode(lua_State* L)
{
    luaL_newmetatable(L, "Timecode");

    lua_pushvalue(L, -1);
    lua_setuserdatametatable(L, kTimecodeUserdataTag);

    lua_pushstring(L, "The metatable is locked");
    lua_setfield(L, -2, "__metatable");

    lua_pushstring(L, "timecode");
    lua_setfield(L, -2, "__type");

    lua_pushcfunction(L, timecode::index, "__index");
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, timecode::tostring, "__tostring");
    lua_setfield(L, -2, "__tostring");

    lua_setreadonly(L, -1, true);
    lua_pop(L, 1);

    return 0;
}

int adoreopen_timecode(lua_State* L)
{
    lua_createtable(L, 0, std::size(timecode::lib));

    for (auto& [name, func] : timecode::lib)
    {
        if (!name || !func)
            break;

        lua_pushcfunction(L, func, name);
        lua_setfield(L, -2, name);
    }

    lua_setreadonly(L, -1, true);
    adoreregister_timecode(L);

    return 1;
}

