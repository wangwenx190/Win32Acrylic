cmake_minimum_required(VERSION 3.21.2 FATAL_ERROR)

project(WinUI2Demo LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "") # TODO: make it 20 once the upstream bug is fixed
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "")
set(CMAKE_CXX_EXTENSIONS OFF CACHE BOOL "")

if(NOT ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug"))
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON CACHE BOOL "")
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)

set(CMAKE_CXX_STANDARD_LIBRARIES "Kernel32.lib User32.lib" CACHE STRING "" FORCE)

set(SOURCES
    WinUI2.rc
    WinUI2.manifest
    Resource.h
    RuntimeObject.cpp
    WinUI2.cpp
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
    /await # TODO: /await:strict
    #/permissive-
    /MP
    /EHsc
    /GR
    /d2FH4
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
        /guard:ehcont
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        /DYNAMICBASE
        /NXCOMPAT
        /HIGHENTROPYVA
        /LARGEADDRESSAWARE
        /INCREMENTAL:NO
        /OPT:REF
        /OPT:ICF
        /GUARD:CF
        /CETCOMPAT
    )
endif()
