
add_executable(Adore.CLI)

target_sources(Adore.CLI PRIVATE
    src/main.cpp
)


set_target_properties(Adore.CLI PROPERTIES OUTPUT_NAME adore)
target_compile_features(Adore.CLI PUBLIC cxx_std_17)
target_link_libraries(Adore.CLI PRIVATE Lute.CLI.lib Lute.Runtime Lute.Require Luau.Common Luau.Compiler Luau.Config Luau.CodeGen Luau.Analysis Luau.VM Luau.CLI.lib raylib Adore.Window Adore.Graphics)
target_compile_options(Adore.CLI PRIVATE ${LUTE_OPTIONS})

