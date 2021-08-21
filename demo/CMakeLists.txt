enable_language(RC)

set(SOURCES_COMMON
    demo.manifest
    demo.rc
    am_apis.hpp
)

set(SOURCES_SIMPLE
    ${SOURCES_COMMON}
    simple.cpp
)

set(SOURCES_HOSTWINDOW
    ${SOURCES_COMMON}
    hostwindow.cpp
)

add_executable(Demo_Simple WIN32 ${SOURCES_SIMPLE})
add_executable(Demo_HostWindow WIN32 ${SOURCES_HOSTWINDOW})

target_link_libraries(Demo_HostWindow PRIVATE
    user32
)

target_compile_definitions(Demo_Simple PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    _CRT_SECURE_NO_WARNINGS
)
target_compile_definitions(Demo_HostWindow PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    _CRT_SECURE_NO_WARNINGS
)

if(MSVC)
    set(COMPILE_OPTIONS_COMMON /utf-8 /permissive-)
    set(LINK_OPTIONS_COMMON)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(COMPILE_OPTIONS_COMMON ${COMPILE_OPTIONS_COMMON} /JMC)
    else()
        set(COMPILE_OPTIONS_COMMON ${COMPILE_OPTIONS_COMMON} /guard:cf)
        set(LINK_OPTIONS_COMMON ${LINK_OPTIONS_COMMON} /GUARD:CF)
    endif()
    target_compile_options(Demo_Simple PRIVATE ${COMPILE_OPTIONS_COMMON})
    target_compile_options(Demo_HostWindow PRIVATE ${COMPILE_OPTIONS_COMMON})
    target_link_options(Demo_Simple PRIVATE ${LINK_OPTIONS_COMMON})
    target_link_options(Demo_HostWindow PRIVATE ${LINK_OPTIONS_COMMON})
endif()
