enable_language(RC)

set(SOURCES
    demo.manifest
    demo.rc
    main.cpp
)

add_executable(Demo WIN32 ${SOURCES})

target_link_libraries(Demo PRIVATE
    wangwenx190::AcrylicManager
)

if(MSVC)
    target_compile_options(Demo PRIVATE /utf-8 /await:strict /permissive-)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(Demo PRIVATE /JMC)
    else()
        target_compile_options(Demo PRIVATE /guard:cf)
        target_link_options(Demo PRIVATE /GUARD:CF)
    endif()
endif()
