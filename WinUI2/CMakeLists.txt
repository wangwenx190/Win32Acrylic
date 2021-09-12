cmake_minimum_required(VERSION 3.21.2 FATAL_ERROR)

project(WinUI2Demo LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 17) # TODO: make it 20 once the upstream bug is fixed
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug"))
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()

set(SOURCES
    WinUI2.rc
    WinUI2.manifest
    resource.h
    RuntimeObject.cpp
    main.cpp
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

set(NTDDI_WIN10_19H1 0x0A000007)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    WIN32_LEAN_AND_MEAN
    WINRT_LEAN_AND_MEAN
    UNICODE
    _UNICODE
    WINVER=${NTDDI_WIN10_19H1}
    _WIN32_WINNT=${NTDDI_WIN10_19H1}
    NOMINMAX
    _CRT_NON_CONFORMING_SWPRINTFS
    _CRT_SECURE_NO_WARNINGS
)
unset(NTDDI_WIN10_19H1)

target_compile_options(${PROJECT_NAME} PRIVATE
    /utf-8
    /Zc:__cplusplus
    /await # TODO: /await:Strict
    /permissive-
)
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE
        /JMC
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        /Gy
        /Zc:inline
        /guard:cf
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        /DYNAMICBASE
        /NXCOMPAT
        /OPT:REF
        /OPT:ICF
        /INCREMENTAL:NO
        /GUARD:CF
    )
endif()
