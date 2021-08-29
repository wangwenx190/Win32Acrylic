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

set(NTDDI_WIN10_19H1 0x0A000007)
target_compile_definitions(Demo PRIVATE
    WIN32_LEAN_AND_MEAN
    WINRT_LEAN_AND_MEAN
    UNICODE
    _UNICODE
    WINVER=${NTDDI_WIN10_19H1}
    _WIN32_WINNT=${NTDDI_WIN10_19H1}
    _ENABLE_EXTENDED_ALIGNED_STORAGE
    NOMINMAX
    _CRT_NON_CONFORMING_SWPRINTFS
    _CRT_SECURE_NO_WARNINGS
)
unset(NTDDI_WIN10_19H1)
