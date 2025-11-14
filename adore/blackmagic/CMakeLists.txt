
add_library(Adore.Blackmagic STATIC)

target_sources(Adore.Blackmagic PRIVATE
    include/adore/blackmagic.h
    
    src/blackmagic.cpp
)

set_target_properties(Adore.Blackmagic PROPERTIES OUTPUT_NAME adore)
target_include_directories(Adore.Blackmagic PUBLIC "include")
target_compile_features(Adore.Blackmagic PUBLIC cxx_std_17)
target_link_libraries(Adore.Blackmagic PRIVATE Luau.VM uSocket)
target_compile_options(Adore.Blackmagic PRIVATE ${LUTE_OPTIONS})

IF (WIN32)
    add_library(BlackmagicSwitcher STATIC stub.cpp)
    SET(BLACKMAGIC_HEADER_DIR "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/BlackmagicSwitcher.dir/")
    target_include_directories(BlackmagicSwitcher PUBLIC
        "${CMAKE_CURRENT_BINARY_DIR}/extern/blackmagic/"
        ${BLACKMAGIC_HEADER_DIR}
    )

    SET(MIDL_INPUT_IDL "${CMAKE_CURRENT_SOURCE_DIR}/../../extern/blackmagic/Windows/include/BMDSwitcherAPI.idl")
    SET(MIDL_OUTPUT_TLB "${CMAKE_CURRENT_BINARY_DIR}/extern/blackmagic/BMDSwitcherAPI.tlb")
    SET(MIDL_MACHINE_TYPE "amd64")
    
    add_custom_target(GenerateTLB
        BYPRODUCTS ${MIDL_OUTPUT_TLB}
        COMMAND midl /env ${MIDL_MACHINE_TYPE} /tlb ${MIDL_OUTPUT_TLB} ${MIDL_INPUT_IDL}
        COMMENT "Generating Blackmagic Switcher API files with MIDL"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    add_dependencies(BlackmagicSwitcher GenerateTLB)

    add_custom_target(GenerateTLH
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/BlackmagicSwitcher.dir/src/BMDSwitcherAPI.tlh"
        COMMAND echo "Generating Blackmagic Switcher API Tlh file"
    )
    add_dependencies(GenerateTLH BlackmagicSwitcher)

    add_dependencies(Adore.Blackmagic GenerateTLH)

    target_include_directories(Adore.Blackmagic PRIVATE ${BLACKMAGIC_HEADER_DIR})
    target_link_libraries(Adore.Blackmagic PRIVATE BlackmagicSwitcher)
ELSE()
    message(WARNING "Blackmagic library is only supported on Windows (right now).")
endif()
