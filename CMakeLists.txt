cmake_minimum_required(VERSION 3.25)

project(adore VERSION 0.1.0 LANGUAGES C CXX)

OPTION(ADORE_BLACKMAGIC "Enable blackmagic design library" ON)

add_subdirectory(extern/lute)
add_subdirectory(extern/raylib)

add_subdirectory(adore/window)
add_subdirectory(adore/graphics)
add_subdirectory(adore/gui)
add_subdirectory(adore/input)
add_subdirectory(adore/cli)

IF (ADORE_BLACKMAGIC)
    add_subdirectory(adore/blackmagic)
ENDIF()
