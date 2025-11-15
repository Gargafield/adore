#include "adore/hyperdeck.h"

#include "lute/runtime.h"
#include <iostream>

#include <memory>
#include <condition_variable>
#include <string>
#include <variant>

#include <uv.h>
#include "BMDSwitcherAPI.tlh"

namespace hyperdeck {

enum HyperdeckState {
    HYPERDECK_STATE_DISCONNECTED,
    HYPERDECK_STATE_CONNECTING,
    HYPERDECK_STATE_CONNECTED,
    HYPERDECK_STATE_ERROR
};

struct HyperdeckDevice {
    lua_State* L;
    ResumeToken token;
    std::string buffer = "";
    HyperdeckState state = HYPERDECK_STATE_DISCONNECTED;
    std::string lastError = "";
    std::condition_variable cv;
    uv_loop_t* loop = nullptr;

    uv_tcp_t* tcp = nullptr;
    uv_connect_t* connect_req = nullptr;

    void processBuffer();

    static std::variant<HyperdeckDevice*, std::string> create(Runtime* runtime, const char* address);

    void close();
};

void HyperdeckDevice::processBuffer() {
    size_t pos = 0;
    while ((pos = buffer.find("\r\n")) != std::string::npos) {
        std::string line = buffer.substr(0, pos);
        buffer.erase(0, pos + 2);

        // Process the line (for demonstration, we just print it)
        printf("Received line: %s\n", line.c_str());
    }
}

void HyperdeckDevice::close() {
    uv_read_stop((uv_stream_t*)tcp);
    uv_close((uv_handle_t*)tcp, [](uv_handle_t* handle) {
        delete reinterpret_cast<uv_tcp_t*>(handle);
    });
    delete connect_req;
    delete this;
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
    }
    delete[] buf->base;
}

static void on_connect(uv_connect_t* req, int status) {
    HyperdeckDevice* device = static_cast<HyperdeckDevice*>(req->handle->data);
    if (status < 0) {
        device->state = HYPERDECK_STATE_ERROR;
        device->lastError = uv_strerror(status);
        device->cv.notify_all();
        return;
    }
    device->state = HYPERDECK_STATE_CONNECTED;
    device->cv.notify_all();
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

    std::cout << "Connected to Hyperdeck at " << address << std::endl;

    if (device->state == HYPERDECK_STATE_ERROR) {
        std::string err = device->lastError;
        delete device->tcp;
        delete device->connect_req;
        delete device;
        return err;
    }

    return device;
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
    HyperdeckDevice** devicePtr = static_cast<HyperdeckDevice**>(lua_touserdatatagged(L, 1, kHyperdeckDeviceUserdataTag));
    if (devicePtr && *devicePtr) {
        HyperdeckDevice* device = *devicePtr;
        device->close();
        *devicePtr = nullptr;
    }
    return 0;
}

int send(lua_State* L) {
    HyperdeckDevice** devicePtr = static_cast<HyperdeckDevice**>(lua_touserdatatagged(L, 1, kHyperdeckDeviceUserdataTag));
    const char* command = luaL_checkstring(L, 2);
    if (devicePtr && *devicePtr) {
        HyperdeckDevice* device = *devicePtr;
        std::string cmdStr = std::string(command) + "\r\n";
        uv_buf_t buf;
        buf.base = const_cast<char*>(cmdStr.c_str());
        buf.len = cmdStr.size();
        uv_write_t* write_req = new uv_write_t();
        uv_write(write_req, (uv_stream_t*)device->tcp, &buf, 1, [](uv_write_t* req, int status) {
            delete req;
        });
    }
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

    lua_pushnil(L);
    return 1;
}

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

    lua_setreadonly(L, -1, false);

    adoreregister_hyperdeck(L);

    return 1;
}

