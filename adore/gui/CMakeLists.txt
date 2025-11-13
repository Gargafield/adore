
add_library(Adore.Gui STATIC)

target_sources(Adore.Gui PRIVATE
    include/adore/gui.h
    
    src/gui.cpp
)

set_target_properties(Adore.Gui PROPERTIES OUTPUT_NAME adore)
target_include_directories(Adore.Gui PUBLIC "include")
target_include_directories(Adore.Gui PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../extern/raygui/src/")
target_compile_features(Adore.Gui PUBLIC cxx_std_17)
target_link_libraries(Adore.Gui PRIVATE Luau.VM Adore.Graphics raylib)
target_compile_options(Adore.Gui PRIVATE ${LUTE_OPTIONS})
