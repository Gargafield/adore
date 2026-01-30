
add_library(Adore.Graphics STATIC)

target_sources(Adore.Graphics PRIVATE
    include/adore/graphics.h
    include/adore/colors.h
    include/adore/texture.h
    include/adore/image.h
    include/adore/rect.h
    include/adore/font.h
    include/adore/rendertexture.h

    src/graphics.cpp
    src/colors.cpp
    src/texture.cpp
    src/image.cpp
    src/rect.cpp
    src/font.cpp
    src/rendertexture.cpp
)

set_target_properties(Adore.Graphics PROPERTIES OUTPUT_NAME adore)
target_include_directories(Adore.Graphics PUBLIC "include")
target_compile_features(Adore.Graphics PUBLIC cxx_std_17)
target_link_libraries(Adore.Graphics PRIVATE Adore.Core Luau.VM Adore.Window raylib)
target_compile_options(Adore.Graphics PRIVATE ${LUTE_OPTIONS})
