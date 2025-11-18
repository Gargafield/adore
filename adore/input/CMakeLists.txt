
add_library(Adore.Input STATIC)

target_sources(Adore.Input PRIVATE
    include/adore/input.h
    
    src/input.cpp
)

set_target_properties(Adore.Input PROPERTIES OUTPUT_NAME adore)
target_include_directories(Adore.Input PUBLIC "include")
target_compile_features(Adore.Input PUBLIC cxx_std_17)
target_link_libraries(Adore.Input PRIVATE Luau.VM raylib)
target_compile_options(Adore.Input PRIVATE ${LUTE_OPTIONS})

