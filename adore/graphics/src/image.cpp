#include "adore/image.h"

#include "adore/window.h"
#include <memory>
#include <iostream>
#include "raylib.h"

namespace image {

int create_image_userdata(lua_State* L, const Image& image) {
    Image* imagePtr = static_cast<Image*>(lua_newuserdatatagged(L, sizeof(Image), kImageUserdataTag));
    *imagePtr = image;

    lua_getuserdatametatable(L, kImageUserdataTag);
    lua_setmetatable(L, -2);

    return 1;
}

int load_image(lua_State* L) {
    WINDOW_NOT_INITIALIZED_CHECK();

    const char* path = luaL_checkstring(L, 1);

    Image image = LoadImage(path);
    if (image.data == NULL) {
        lua_pushnil(L);
        return 1;
    }

    return create_image_userdata(L, image);
}

Image* check_image(lua_State* L, int index) {
    void* ud = lua_touserdatatagged(L, index, kImageUserdataTag);
    if (!ud) {
        luaL_typeerror(L, index, "Image");
    }
    return static_cast<Image*>(ud);
}

static const std::pair<const char*, int> formats[] = {
    { "grayscale", PIXELFORMAT_UNCOMPRESSED_GRAYSCALE },
    { "gray_alpha", PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA },
    { "r5g6b5", PIXELFORMAT_UNCOMPRESSED_R5G6B5 },
    { "r8g8b8", PIXELFORMAT_UNCOMPRESSED_R8G8B8 },
    { "r5g5b5a1", PIXELFORMAT_UNCOMPRESSED_R5G5B5A1 },
    { "r4g4b4a4", PIXELFORMAT_UNCOMPRESSED_R4G4B4A4 },
    { "r8g8b8a8", PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 },
    { "r32", PIXELFORMAT_UNCOMPRESSED_R32 },
    { "r32g32b32", PIXELFORMAT_UNCOMPRESSED_R32G32B32 },
    { "r32g32b32a32", PIXELFORMAT_UNCOMPRESSED_R32G32B32A32 },
    { "r16", PIXELFORMAT_UNCOMPRESSED_R16 },
    { "r16g16b16", PIXELFORMAT_UNCOMPRESSED_R16G16B16 },
    { "r16g16b16a16", PIXELFORMAT_UNCOMPRESSED_R16G16B16A16 }
};

int index(lua_State* L) {
    Image* image = check_image(L, 1);
    const char* key = luaL_checkstring(L, 2);

    if (strcmp(key, "width") == 0) {
        lua_pushinteger(L, image->width);
        return 1;
    } else if (strcmp(key, "height") == 0) {
        lua_pushinteger(L, image->height);
        return 1;
    } else if (strcmp(key, "mipmaps") == 0) {
        lua_pushinteger(L, image->mipmaps);
        return 1;
    } else if (strcmp(key, "format") == 0) {
        for (const auto& [name, format] : formats) {
            if (image->format == format) {
                lua_pushstring(L, name);
                return 1;
            }
        }
        lua_pushstring(L, "unknown");
        return 1;
    }

    luaL_error(L, "Attempt to access invalid image property: %s", key);
    return 0;
}

int format_image(lua_State* L) {
    Image* image = check_image(L, 1);
    const char* formatStr = luaL_checkstring(L, 2);

    for (const auto& [name, format] : formats) {
        if (strcmp(formatStr, name) == 0) {
            Image newImage = ImageCopy(*image);
            ImageFormat(&newImage, format);
            return create_image_userdata(L, newImage);
        }
    }

    luaL_error(L, "Invalid pixel format: %s", formatStr);
    return 0;
}

int export_image(lua_State* L) {
    Image* image = check_image(L, 1);
    const char* path = luaL_checkstring(L, 2);

    std::cout << "Exporting image to " << path << "...\n";

    bool success = ExportImage(*image, path);
    lua_pushboolean(L, success);
    return 1;
}

} // namespace image


int adoreregister_image(lua_State* L)
{
    luaL_newmetatable(L, "Image");
    lua_pushvalue(L, -1);
    lua_setuserdatametatable(L, kImageUserdataTag);

    lua_pushcfunction(L, image::index, nullptr);
    lua_setfield(L, -2, "__index");

    lua_setreadonly(L, -1, true);

    lua_setuserdatadtor(L, kImageUserdataTag,
        [](lua_State* L, void* ud)
        {
            Image* image = static_cast<Image*>(ud);
            UnloadImage(*image);
        }
    );

    lua_pop(L, 1);

    lua_newtable(L, 0, std::size(image::lib));
    luaL_register(L, nullptr, image::lib); //
    lua_setreadonly(L, -1, true);

    return 1;
}
