
add_library(Adore.Graphics STATIC)

target_sources(Adore.Graphics PRIVATE
    include/adore/graphics.h
    include/adore/color.h
    include/adore/texture.h

    src/graphics.cpp
    src/color.cpp
    src/texture.cpp
)

set_target_properties(Adore.Graphics PROPERTIES OUTPUT_NAME adore)
target_include_directories(Adore.Graphics PUBLIC "include")
target_compile_features(Adore.Graphics PUBLIC cxx_std_17)
target_link_libraries(Adore.Graphics PRIVATE Luau.VM raylib)
target_compile_options(Adore.Graphics PRIVATE ${LUTE_OPTIONS})

