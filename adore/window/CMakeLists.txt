
add_library(Adore.Window STATIC)

target_sources(Adore.Window PRIVATE
    include/adore/window.h
    
    src/window.cpp
)


set_target_properties(Adore.Window PROPERTIES OUTPUT_NAME adore)
target_include_directories(Adore.Window PUBLIC "include")
target_compile_features(Adore.Window PUBLIC cxx_std_17)
target_link_libraries(Adore.Window PRIVATE Luau.VM raylib)
target_compile_options(Adore.Window PRIVATE ${LUTE_OPTIONS})

