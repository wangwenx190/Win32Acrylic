enable_language(RC)

set(SOURCES
    demo.manifest
    demo.rc
    am_apis.hpp
    main.cpp
)

add_executable(Demo WIN32 ${SOURCES})

target_link_libraries(Demo PRIVATE
    user32
)

target_compile_definitions(Demo PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    _CRT_SECURE_NO_WARNINGS
)

if(MSVC)
    target_compile_options(Demo PRIVATE /utf-8 /permissive-)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(Demo PRIVATE /JMC)
    else()
        target_compile_options(Demo PRIVATE /guard:cf)
        target_link_options(Demo PRIVATE /GUARD:CF)
    endif()
endif()
