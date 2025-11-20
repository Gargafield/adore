#include "adore/hyperdeck.h"

#include "lute/runtime.h"
#include <iostream>

#include <memory>
#include <optional>
#include <condition_variable>
#include <string>
#include <variant>
#include <queue>

#include <uv.h>
#include "BMDSwitcherAPI.tlh"

#define HYPERDECK_DEBUG 0

namespace hyperdeck {

enum HyperdeckState {
    HYPERDECK_STATE_DISCONNECTED,
    HYPERDECK_STATE_CONNECTING,
    HYPERDECK_STATE_CONNECTED,
    HYPERDECK_STATE_ERROR
};

static const std::pair<const char*, HyperdeckState> kHyperdeckStateMap[] = {
    {"disconnected", HyperdeckState::HYPERDECK_STATE_DISCONNECTED},
    {"connecting", HyperdeckState::HYPERDECK_STATE_CONNECTING},
    {"connected", HyperdeckState::HYPERDECK_STATE_CONNECTED},
    {"error", HyperdeckState::HYPERDECK_STATE_ERROR},
    {nullptr, HyperdeckState::HYPERDECK_STATE_DISCONNECTED}
};

enum HyperdeckReadState {
    NONE,
    TRANSPORT_INFO,
    CLIPS_INFO,
    CONNECTION_INFO,
    DEVICE_INFO,
    TIMELINE_POSITION,
    DISPLAY_TIMECODE,
    DISK_LIST
};

enum HyperdeckStatus {
    HYPERDECK_STATUS_PREVIEW,
    HYPERDECK_STATUS_STOPPED,
    HYPERDECK_STATUS_PLAY,
    HYPERDECK_STATUS_FORWARD,
    HYPERDECK_STATUS_REWIND,
    HYPERDECK_STATUS_JOG,
    HYPERDECK_STATUS_SHUTTLE,
    HYPERDECK_STATUS_RECORD,
};

static const std::pair<const char*, HyperdeckStatus> kHyperdeckStatusMap[] = {
    {"preview", HYPERDECK_STATUS_PREVIEW},
    {"stopped", HYPERDECK_STATUS_STOPPED},
    {"play", HYPERDECK_STATUS_PLAY},
    {"forward", HYPERDECK_STATUS_FORWARD},
    {"rewind", HYPERDECK_STATUS_REWIND},
    {"jog", HYPERDECK_STATUS_JOG},
    {"shuttle", HYPERDECK_STATUS_SHUTTLE},
    {"record", HYPERDECK_STATUS_RECORD},
};

struct TransportInfo {
    HyperdeckStatus status = HYPERDECK_STATUS_STOPPED;
    std::optional<int> slot_id;
    std::optional<std::string> slot_name;
    std::optional<std::string> device_name;
    std::optional<int> clip_id;
    std::optional<bool> single_clip;
    std::optional<std::string> display_timecode;
    std::optional<std::string> timecode;
    std::optional<std::string> video_format;
};

struct ClipInfo {
    int clip_id;
    std::string name;
    std::string duration;
    std::string timecode;
    std::string format;
};

struct DeviceInfo {
    std::string model;
    std::string protocol_version;
    std::string unique_id;
    int slot_count;
    std::string software_version;
    std::string name;
    std::vector<ClipInfo> clips;
};

struct ClipTimelineInfo {
    int clip_id;
    std::string start_timecode;
    std::string duration;
    std::string name;
};

struct TimelineInfo {
    std::string timecode;
    int position;
    int speed;
    std::vector<ClipTimelineInfo> clips;
    std::optional<int> clip_id;
    bool loop = false;
};

struct ProtocolReader;

// define response callback for queue, takes an int code and string message
using ResponseCallback = std::function<void(int, const std::string&)>;

struct HyperdeckDevice {
    lua_State* L;
    ResumeToken token;
    std::string buffer = "";
    HyperdeckState state = HYPERDECK_STATE_DISCONNECTED;
    HyperdeckReadState readState = NONE;

    std::shared_ptr<ProtocolReader> protocolReader = nullptr;

    TransportInfo transportInfo;
    DeviceInfo deviceInfo;
    TimelineInfo timelineInfo;
    bool ready = false;

    std::string lastError = "";
    std::recursive_mutex mutex;
    std::queue<ResponseCallback> callbackQueue;

    uv_loop_t* loop = nullptr;
    uv_tcp_t* tcp = nullptr;
    uv_connect_t* connect_req = nullptr;

    void processBuffer();

    static std::variant<HyperdeckDevice*, std::string> create(Runtime* runtime, const char* address);

    void close();
    void send(const std::string& command);
    void sendWithCallback(const std::string& command, ResponseCallback callback);
};

void HyperdeckDevice::close() {
    uv_read_stop((uv_stream_t*)tcp);
    if (state == HYPERDECK_STATE_CONNECTED) {
        send("quit\r\n");
    }
    uv_close((uv_handle_t*)tcp, [](uv_handle_t* handle) {
        delete reinterpret_cast<uv_tcp_t*>(handle);
    });
    delete connect_req;
    delete this;
}

void HyperdeckDevice::send(const std::string& command) {
    this->sendWithCallback(command, [](int a, const std::string& b){ });
}

void HyperdeckDevice::sendWithCallback(const std::string& command, ResponseCallback callback) {
    struct WriteRequest {
        uv_write_t req;
        std::string buffer;
        std::recursive_mutex* mutex;
    };

    auto* write_req = new WriteRequest();
    write_req->buffer = command; // Copy the command string
    write_req->mutex = &this->mutex;

    write_req->mutex->lock();
    callbackQueue.push(callback);

    uv_buf_t buf;
    buf.base = write_req->buffer.data();
    buf.len = write_req->buffer.size();
    write_req->req.data = write_req;

    uv_write(&write_req->req, (uv_stream_t*)tcp, &buf, 1, [](uv_write_t* req, int status) {
        auto* write_req = static_cast<WriteRequest*>(req->data);
        write_req->mutex->unlock();
        delete write_req;
    });
}

std::vector<std::string_view> splitString(const std::string& str, char delimiter) {
    std::vector<std::string_view> parts;
    size_t start = 0;
    size_t end = str.find(delimiter);
    while (end != std::string::npos) {
        parts.emplace_back(str.data() + start, end - start);
        start = end + 1;
        end = str.find(delimiter, start);
    }
    parts.emplace_back(str.data() + start, str.size() - start);
    return parts;
}

struct ProtocolReader {
    virtual void start(HyperdeckDevice* device) = 0;
    virtual void processLine(HyperdeckDevice* device, const std::string& line) = 0;
    virtual void finish(HyperdeckDevice* device) = 0;
};

static const std::tuple<const char*, HyperdeckReadState> kHyperdeckReadStateMap[] = {
    {"transport info", HyperdeckReadState::TRANSPORT_INFO},
    {"clips info", HyperdeckReadState::CLIPS_INFO},
    {"connection info", HyperdeckReadState::CONNECTION_INFO},
    {"device info", HyperdeckReadState::DEVICE_INFO},
    {"timeline position", HyperdeckReadState::TIMELINE_POSITION},
    {"display timecode", HyperdeckReadState::DISPLAY_TIMECODE},
    {"disk list", HyperdeckReadState::DISK_LIST},
    {"disk list info", HyperdeckReadState::DISK_LIST},
    {nullptr, HyperdeckReadState::NONE}
};

#define PROTOCOL_KEY(info, x, expr) if (key == #x) { \
    device->info.x = expr; \
}

struct TransportInfoReader : public ProtocolReader {
    void start(HyperdeckDevice* device) override { }

    void processLine(HyperdeckDevice* device, const std::string& line) override {
        // first part until ':*, value after
        auto pos = line.find(':');
        if (pos == std::string::npos) {
            return;
        }
        auto key = line.substr(0, pos);
        auto value = line.substr(pos + 2);
        if (key == "status") {
            for (const auto& [name, status] : kHyperdeckStatusMap) {
                if (value == name) {
                    device->transportInfo.status = status;
                    break;
                }
            }
        } else if (key == "speed") {
            device->timelineInfo.speed = std::stoi(value);
        } else if (key == "loop") {
            device->timelineInfo.loop = (value == "true");
        } else if (key == "timeline") {
            device->timelineInfo.position = std::stoi(value);
        } else if (key == "slot id") {
            device->transportInfo.slot_id = std::stoi(value);
        } else if (key == "slot name") {
            device->transportInfo.slot_name = value;
        } else if (key == "device name") {
            device->transportInfo.device_name = value;
        } else if (key == "clip id") {
            device->timelineInfo.clip_id = std::stoi(value);
        } else if (key == "single clip") {
            device->transportInfo.single_clip = (value == "true");
        } else if (key == "display timecode") {
            device->timelineInfo.timecode = value;
        } else if (key == "timecode") {
            device->transportInfo.timecode = value;
        } else if (key == "video format") {
            device->transportInfo.video_format = value;
        }
    }

    void finish(HyperdeckDevice* device) override {
        // No special finalization needed
    }
};

struct DiskListReader : public ProtocolReader {
    std::vector<ClipInfo> clips;
    bool isSnapshot = false;
    void start(HyperdeckDevice* device) override { }

    void processLine(HyperdeckDevice* device, const std::string& line) override {
        if (line._Starts_with("slot id")) {
            // ignore
            return;
        } else if (line._Starts_with("update type: snapshot")) {
            this->isSnapshot = true;
            // copy existing clips from device
            this->clips = device->deviceInfo.clips;
            return;
        } else if (line._Starts_with("clip count")) {
            // Set length of clipsInfo vector
            int clipCount = std::stoi(line.substr(11));
            this->clips.resize(clipCount);
            return;
        }

        // 1: blow-me_2.mov QuickTimeProResLT 1080p60 00:00:05:00
        auto parts = splitString(line, ' ');
        if (parts.size() < 4) {
            if (HYPERDECK_DEBUG) {
                std::cout << "Invalid clip info line: " << line << std::endl;
            }
            return;
        }

        ClipInfo clip;
        // Remove trailing colon from id
        clip.clip_id = std::stoi(std::string(parts[0].substr(0, parts[0].length() - 1)));
        clip.name = std::string(parts[1]);
        clip.format = std::string(parts[3]);
        clip.duration = std::string(parts[4]);

        this->clips.push_back(clip);
    }

    void finish(HyperdeckDevice* device) override {
        device->deviceInfo.clips = this->clips;
    }
};

struct ClipInfoReader : public ProtocolReader {
    std::vector<ClipTimelineInfo> clips;
    bool isSnapshot = false;
    void start(HyperdeckDevice* device) override {
        clips.clear();
    }

    void processLine(HyperdeckDevice* device, const std::string& line) override {
        if (line._Starts_with("clip count")) {
            // Set length of clipsInfo vector
            int clipCount = std::stoi(line.substr(11));
            this->clips.reserve(clipCount);
            return;
        } else if (line._Starts_with("update type: snapshot")) {
            this->isSnapshot = true;
            return;
        }

        auto parts = splitString(line, ' ');

        ClipTimelineInfo clip;
        // Remove trailing colon from i
        clip.clip_id = std::stoi(std::string(parts[0].substr(0, parts[0].length() - 1)));
        if (parts.size() == 4) {
            clip.name = std::string(parts[1]);
            clip.start_timecode = std::string(parts[2]);
            clip.duration = std::string(parts[3]);
        } else if (parts.size() == 6) {
            clip.start_timecode = std::string(parts[1]);
            clip.duration = std::string(parts[2]);
            clip.name = std::string(parts[5]);
        }
        
        this->clips.push_back(clip);
    }

    void finish(HyperdeckDevice* device) override {
        device->timelineInfo.clips = this->clips;
    }
};

struct ConnectionInfoReader : public ProtocolReader {
    void start(HyperdeckDevice* device) override { }

    void processLine(HyperdeckDevice* device, const std::string& line) override {
        if (line._Starts_with("protocol version")) {
            device->deviceInfo.protocol_version = line.substr(18);
            return;
        } else if (line._Starts_with("model")) {
            device->deviceInfo.model = line.substr(7);
            return;
        }
    }

    void finish(HyperdeckDevice* device) override {
        device->ready = true;
    }
};

struct DeviceInfoReader : public ProtocolReader {
    void start(HyperdeckDevice* device) override { }

    void processLine(HyperdeckDevice* device, const std::string& line) override {
        if (line._Starts_with("unique id")) {
            device->deviceInfo.unique_id = line.substr(10);
            return;
        } else if (line._Starts_with("slot count")) {
            device->deviceInfo.slot_count = std::stoi(line.substr(11));
            return;
        } else if (line._Starts_with("software version")) {
            device->deviceInfo.software_version = line.substr(17);
            return;
        } else if (line._Starts_with("name")) {
            device->deviceInfo.name = line.substr(5);
            return;
        }
    }

    void finish(HyperdeckDevice* device) override {
    }
};

void HyperdeckDevice::processBuffer() {
    size_t pos = 0;
    while ((pos = buffer.find("\r\n")) != std::string::npos) {
        std::string line = buffer.substr(0, pos);
        if (HYPERDECK_DEBUG) {
            std::cout << "Received line: " << line << std::endl;
        }
        buffer.erase(0, pos + 2);
        if (line.empty()) {
            if (this->protocolReader.get() != nullptr) {
                this->protocolReader->finish(this);
                this->protocolReader = nullptr;
            }
            this->readState = NONE;
            continue;
        }
        
        auto parts = splitString(line, ' ');
        if (this->readState == NONE) {
            int code;
            if (sscanf(parts[0].data(), "%d", &code) != 1) {
                // invalid line
                continue;
            }

            // rest of the line is the key ignoring the colon
            std::string key = std::string(line.substr(4));
            if (key.back() == ':') {
                key.pop_back();
            }

            // Call the callback for this response if any
            if (code >= 100 && code < 500) {
                // Ignore async notifications
                std::lock_guard<std::recursive_mutex> lock(this->mutex);
                if (!this->callbackQueue.empty()) {
                    ResponseCallback callback = this->callbackQueue.front();
                    this->callbackQueue.pop();
                    callback(code, key);
                }
            }

            if (key == "ok") {
                // ignore
                continue;
            }

            for (const auto& [name, state] : kHyperdeckReadStateMap) {
                if (name && key == name) {
                    this->readState = state;
                    switch (this->readState) {
                        case TRANSPORT_INFO:
                        case TIMELINE_POSITION:
                        case DISPLAY_TIMECODE:
                            this->protocolReader = std::make_shared<TransportInfoReader>();
                            break;
                        case DISK_LIST:
                            this->protocolReader = std::make_shared<DiskListReader>();
                            break;
                        case CLIPS_INFO:
                            this->protocolReader = std::make_shared<ClipInfoReader>();
                            break;
                        case CONNECTION_INFO:
                            this->protocolReader = std::make_shared<ConnectionInfoReader>();
                            break;
                        case DEVICE_INFO:
                            this->protocolReader = std::make_shared<DeviceInfoReader>();
                            break;
                        default:
                            this->protocolReader = nullptr;
                            break;
                    }
                    if (this->protocolReader.get() != nullptr) {
                        this->protocolReader->start(this);
                    }
                    break;
                }
            }

            if (HYPERDECK_DEBUG && this->protocolReader.get() == nullptr) {
                std::cout << "No protocol reader for key \"" << key << "\"" << std::endl;
            }

        } else {
            if (this->protocolReader) {
                this->protocolReader->processLine(this, line);
            }
        }
    }
}

static void on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    buf->base = new char[suggested_size];
    buf->len = suggested_size;
}

static void on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    HyperdeckDevice* device = static_cast<HyperdeckDevice*>(stream->data);
    if (nread > 0) {
        device->buffer.append(buf->base, nread);
        device->processBuffer();
    } else if (nread < 0) {
        // disconnect or error
        device->state = HYPERDECK_STATE_DISCONNECTED;
        device->ready = false;
    }
    delete[] buf->base;
}

static void on_connect(uv_connect_t* req, int status) {
    HyperdeckDevice* device = static_cast<HyperdeckDevice*>(req->handle->data);
    if (status < 0) {
        device->state = HYPERDECK_STATE_ERROR;
        device->lastError = uv_strerror(status);
        return;
    }
    device->state = HYPERDECK_STATE_CONNECTED;
    uv_read_start((uv_stream_t*)req->handle, on_alloc, on_read);
    
}

std::variant<HyperdeckDevice*, std::string> HyperdeckDevice::create(Runtime* runtime, const char* address) {
    HyperdeckDevice* device = new HyperdeckDevice();
    device->tcp = new uv_tcp_t();
    uv_tcp_init(uv_default_loop(), device->tcp);
    device->tcp->data = device;
    device->connect_req = new uv_connect_t();
    device->connect_req->data = device;
    struct sockaddr_in dest;
    int r = uv_ip4_addr(address, HYPERDECK_PORT, &dest);
    if (r < 0) {
        delete device->connect_req;
        delete device->tcp;
        delete device;
        return std::string("Invalid address: ") + uv_strerror(r);
    }

    device->state = HYPERDECK_STATE_CONNECTING;
    r = uv_tcp_connect(device->connect_req, device->tcp, (const struct sockaddr*)&dest, on_connect);
    if (r < 0) {
        delete device->connect_req;
        delete device->tcp;
        delete device;
        return std::string("Failed to initiate connection: ") + uv_strerror(r);
    }

    while (device->state == HYPERDECK_STATE_CONNECTING) {
        uv_run(uv_default_loop(), UV_RUN_NOWAIT);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (device->state == HYPERDECK_STATE_ERROR) {
        std::string err = device->lastError;
        delete device->tcp;
        delete device->connect_req;
        delete device;
        return err;
    }

    return device;
}

static HyperdeckDevice* luaL_checkhyperdeck(lua_State* L, int index) {
    HyperdeckDevice** devicePtr = static_cast<HyperdeckDevice**>(lua_touserdatatagged(L, index, kHyperdeckDeviceUserdataTag));
    if (!devicePtr || !*devicePtr) {
        luaL_error(L, "Invalid Hyperdeck device");
        return nullptr;
    }
    return *devicePtr;
}

int connect(lua_State* L) {
    const char* address = luaL_checkstring(L, 1);

    HyperdeckDevice* device = new HyperdeckDevice();
    device->L = L;
    device->token = getResumeToken(L);
    device->loop = uv_default_loop();

    device->tcp = new uv_tcp_t();
    uv_tcp_init(uv_default_loop(), device->tcp);
    device->tcp->data = device;
    device->connect_req = new uv_connect_t();
    device->connect_req->data = device;
    sockaddr_in sockaddr;
    uv_ip4_addr(address, HYPERDECK_PORT, &sockaddr);

    uv_tcp_connect(
        device->connect_req,
        device->tcp,
        (const struct sockaddr*)&sockaddr,
        [](uv_connect_t* req, int status) {
            HyperdeckDevice* device = static_cast<HyperdeckDevice*>(req->data);
            if (status < 0) {
                device->state = HYPERDECK_STATE_ERROR;
                device->token->fail(std::string("Failed to connect: ") + uv_strerror(status));
                device->close();
            } else {
                device->state = HYPERDECK_STATE_CONNECTED;

                device->send("notify:\r\ntransport: true\r\nslot: false\r\nremote: true\r\nconfiguration: true\r\ndropped frames: true\r\ndisplay timecode: true\r\ntimeline position: true\r\nplayrange: true\r\ncache: true\r\ndynamic range: true\r\nslate: false\r\nclips: true\r\ndisk: true\r\ndevice info: true\r\nnas: false\r\n\r\n");
                device->send("device info\r\n");
                device->send("transport info\r\n");
                device->send("playrange\r\n");
                device->send("clips get\r\n");
                device->send("play option\r\n");
                device->send("disk list\r\n");

                device->token->complete(
                    [device](lua_State* L) {
                        HyperdeckDevice** ud = static_cast<HyperdeckDevice**>(lua_newuserdatatagged(L, sizeof(HyperdeckDevice*), kHyperdeckDeviceUserdataTag));
                        *ud = device;

                        lua_getuserdatametatable(L, kHyperdeckDeviceUserdataTag);
                        lua_setmetatable(L, -2);
                        return 1;
                    }
                );
                uv_read_start((uv_stream_t*)req->handle, on_alloc, on_read);
            }
        }
    );

    return lua_yield(L, 0);
}

int close(lua_State* L) {
    HyperdeckDevice* device = luaL_checkhyperdeck(L, 1);
    device->close();
    return 0;
}

int send(lua_State* L) {
    HyperdeckDevice* device = luaL_checkhyperdeck(L, 1);
    const char* command = luaL_checkstring(L, 2);
    device->send(std::string(command) + "\r\n");
    return 0;
}

int clear(lua_State* L) {
    HyperdeckDevice* device = luaL_checkhyperdeck(L, 1);
    
    auto token = getResumeToken(L);
    device->sendWithCallback("clips clear\r\n",
        [token](int code, const std::string& message) {
            if (code >= 200 && code < 300) {
                token->complete([](lua_State* L) {
                    return 0;
                });
            } else {
                token->fail("Failed to clear clips: " + message);
            }
        }
    );

    return lua_yield(L, 0);
}

int addclip(lua_State* L) {
    HyperdeckDevice* device = luaL_checkhyperdeck(L, 1);
    std::string command;
    if (lua_isstring(L, 2)) {
        const char* name = luaL_checkstring(L, 2);
        command = std::string("clips add: name: ") + name + "\r\n";
    } else if (lua_isnumber(L, 2)) {
        int clip_id = luaL_checkinteger(L, 2);
        for (const ClipInfo& clip : device->deviceInfo.clips) {
            if (clip.clip_id == clip_id) {
                command = std::string("clips add: name: ") + clip.name + "\r\n";
                break;
            }
        }
        if (command.empty())
            luaL_error(L, "Clip ID %d not found", clip_id);
    } else {
        luaL_error(L, "Expected string or integer for clip name or ID");
    }

    auto token = getResumeToken(L);
    device->sendWithCallback(command,
        [token](int code, const std::string& message) {
            if (code >= 200 && code < 300) {
                token->complete([](lua_State* L) {
                    return 0;
                });
            } else {
                token->fail("Failed to add clip: " + message);
            }
        }
    );

    return lua_yield(L, 0);
}

int play(lua_State* L) {
    HyperdeckDevice* device = luaL_checkhyperdeck(L, 1);

    if (lua_gettop(L) >= 2) {
        std::string command = "play: ";
        int clipId = luaL_checkinteger(L, 2);
        command += "clip id: " + std::to_string(clipId);

        if (lua_gettop(L) >= 3 && lua_istable(L, 3)) {
            if (lua_getfield(L, 3, "single") == LUA_TBOOLEAN) {
                if (lua_toboolean(L, -1)) {
                    command += " single clip: true";
                } else {
                    command += " single clip: false";
                }
            }
            lua_pop(L, 1);
            if (lua_getfield(L, 3, "loop") == LUA_TBOOLEAN) {
                if (lua_toboolean(L, -1)) {
                    command += " loop: true";
                } else {
                    command += " loop: false";
                }
            }
        }

        command += "\r\n";
        device->send(command);
    } else {
        device->send("play\r\n");
    }

    return 0;
}

int stop(lua_State* L) {
    HyperdeckDevice* device = luaL_checkhyperdeck(L, 1);
    device->send("stop\r\n");
    return 0;
}

int _goto(lua_State* L) {
    HyperdeckDevice* device = luaL_checkhyperdeck(L, 1);
    const char* type = luaL_checkstring(L, 2);

    std::string command;
    if (strcmp(type, "clip") == 0) {
        int clip_id = luaL_checkinteger(L, 3);
        command = "goto: clip id: " + std::to_string(clip_id) + "\r\n";
    } else if (strcmp(type, "timecode") == 0) {
        const char* timecode = luaL_checkstring(L, 3);
        command = std::string("goto: timecode: ") + timecode + "\r\n";
    } else if (strcmp(type, "position") == 0) {
        int position = luaL_checkinteger(L, 3);
        command = "goto: timeline: " + std::to_string(position) + "\r\n";
    } else {
        luaL_error(L, "Invalid goto type: %s", type);
    }

    device->send(command);
    return 0;
}

int index(lua_State* L) {
    const char* key = luaL_checkstring(L, 2);

    for (auto& [name, func] : udata) {
        if (name && strcmp(name, key) == 0) {
            lua_pushcfunction(L, func, name);
            return 1;
        }
    }

    HyperdeckDevice** devicePtr = static_cast<HyperdeckDevice**>(lua_touserdatatagged(L, 1, kHyperdeckDeviceUserdataTag));
    if (!devicePtr || !*devicePtr) {
        lua_pushnil(L);
        return 1;
    }
    HyperdeckDevice* device = *devicePtr;

    if (strcmp(key, "status") == 0) {
        for (const auto& [name, status] : kHyperdeckStatusMap) {
            if (device->transportInfo.status == status) {
                lua_pushstring(L, name);
                break;
            }
        }
        if (!lua_isstring(L, -1)) {
            lua_pushstring(L, "unknown");
        }
        return 1;
    } else if (strcmp(key, "state") == 0) {
        for (const auto& [name, state] : kHyperdeckStateMap) {
            if (device->state == state) {
                lua_pushstring(L, name);
                break;
            }
        }
        if (!lua_isstring(L, -1)) {
            lua_pushstring(L, "unknown");
        }
        return 1;
    } else if (strcmp(key, "clips") == 0) {
        lua_createtable(L, device->deviceInfo.clips.size(), 0);
        for (size_t i = 0; i < device->deviceInfo.clips.size(); ++i) {
            const ClipInfo& clip = device->deviceInfo.clips[i];
            lua_createtable(L, 0, 5);
            lua_pushinteger(L, clip.clip_id);
            lua_setfield(L, -2, "id");
            lua_pushstring(L, clip.name.c_str());
            lua_setfield(L, -2, "name");
            lua_pushstring(L, clip.duration.c_str());
            lua_setfield(L, -2, "duration");
            lua_pushstring(L, clip.timecode.c_str());
            lua_setfield(L, -2, "timecode");
            lua_pushstring(L, clip.format.c_str());
            lua_setfield(L, -2, "format");
            lua_rawseti(L, -2, clip.clip_id);
        }
        return 1;
    } else if (strcmp(key, "timeline") == 0) {
        lua_createtable(L, device->timelineInfo.clips.size(), 0);
        for (size_t i = 0; i < device->timelineInfo.clips.size(); ++i) {
            const ClipTimelineInfo& clip = device->timelineInfo.clips[i];
            lua_createtable(L, 0, 4);
            lua_pushinteger(L, clip.clip_id);
            lua_setfield(L, -2, "id");
            lua_pushstring(L, clip.name.c_str());
            lua_setfield(L, -2, "name");
            lua_pushstring(L, clip.start_timecode.c_str());
            lua_setfield(L, -2, "start");
            lua_pushstring(L, clip.duration.c_str());
            lua_setfield(L, -2, "duration");
            lua_rawseti(L, -2, clip.clip_id);
        }
        return 1;
    } else if (strcmp(key, "model") == 0) {
        lua_pushstring(L, device->deviceInfo.model.c_str());
        return 1;
    } else if (strcmp(key, "name") == 0) {
        lua_pushstring(L, device->deviceInfo.name.c_str());
        return 1;
    } else if (strcmp(key, "ready") == 0) {
        lua_pushboolean(L, device->ready);
        return 1;
    } else if (strcmp(key, "clip") == 0) {
        if (device->timelineInfo.clip_id.has_value()) {
            lua_pushinteger(L, device->timelineInfo.clip_id.value());
        } else {
            lua_pushnil(L);
        }
        return 1;
    } else if (strcmp(key, "timecode") == 0) {
        lua_pushstring(L, device->timelineInfo.timecode.c_str());
        return 1;
    } else if (strcmp(key, "position") == 0) {
        lua_pushinteger(L, device->timelineInfo.position);
        return 1;
    } else if (strcmp(key, "speed") == 0) {
        lua_pushinteger(L, device->timelineInfo.speed);
        return 1;
    } else if (strcmp(key, "loop") == 0) {
        lua_pushboolean(L, device->timelineInfo.loop);
        return 1;
    }
    lua_pushnil(L);
    return 1;
}

struct VideoFormat {
    const char* name;
    int width;
    int height;
    int frameRate;
};

static const VideoFormat kVideoFormats[] = {
    { "720p50", 1280, 720, 50 },
    { "720p5994", 1280, 720, 60 },
    { "720p60", 1280, 720, 60 },
    { "1080p23976", 1920, 1080, 24 },
    { "1080p24", 1920, 1080, 24 },
    { "1080p25", 1920, 1080, 25 },
    { "1080p2997", 1920, 1080, 30 },
    { "1080p30", 1920, 1080, 30 },
    { "1080p60", 1920, 1080, 60 },
    { "1080i50", 1920, 1080, 50 },
    { "1080i5994", 1920, 1080, 60 },
    { "1080i60", 1920, 1080, 60 },
};

} // namespace hyperdeck

static int adoreregister_hyperdeck(lua_State* L)
{
    luaL_newmetatable(L, "HyperdeckDevice");

    lua_pushvalue(L, -1);
    lua_setuserdatametatable(L, kHyperdeckDeviceUserdataTag);

    lua_pushcfunction(L, hyperdeck::index, "__index");
    lua_setfield(L, -2, "__index");

    lua_setreadonly(L, -1, true);
    lua_pop(L, 1);

    lua_setuserdatadtor(L, kHyperdeckDeviceUserdataTag, [](lua_State* L, void* userdata) {
        hyperdeck::HyperdeckDevice** devicePtr = static_cast<hyperdeck::HyperdeckDevice**>(userdata);
        if (devicePtr && *devicePtr) {
            hyperdeck::HyperdeckDevice* device = *devicePtr;
            device->close();
            *devicePtr = nullptr;
        }
    });

    return 0;
}

int adoreopen_hyperdeck(lua_State* L)
{
    lua_createtable(L, 0, std::size(hyperdeck::lib));

    for (auto& [name, func] : hyperdeck::lib)
    {
        if (!name || !func)
            break;

        lua_pushcfunction(L, func, name);
        lua_setfield(L, -2, name);
    }

    lua_createtable(L, 0, std::size(hyperdeck::kVideoFormats));
    for (const auto& format : hyperdeck::kVideoFormats) {
        lua_createtable(L, 0, 3);
        lua_pushinteger(L, format.width);
        lua_setfield(L, -2, "width");
        lua_pushinteger(L, format.height);
        lua_setfield(L, -2, "height");
        lua_pushinteger(L, format.frameRate);
        lua_setfield(L, -2, "framerate");
        lua_pushstring(L, format.name);
        lua_setfield(L, -2, "name");
        lua_setfield(L, -2, format.name);
    }
    lua_setreadonly(L, -1, true);
    lua_setfield(L, -2, "formats");

    lua_setreadonly(L, -1, true);

    adoreregister_hyperdeck(L);

    return 1;
}

