cmake_minimum_required(VERSION 3.21.1 FATAL_ERROR)

project(Win32Acrylic VERSION 1.0.0.0 LANGUAGES CXX RC)

#[[set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)]]

set(CMAKE_CXX_STANDARD 17) # TODO: find out how to support C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT DEFINED CMAKE_DEBUG_POSTFIX)
    if(WIN32)
        set(CMAKE_DEBUG_POSTFIX d)
    else()
        set(CMAKE_DEBUG_POSTFIX _debug)
    endif()
endif()

if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()

if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()

if(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
endif()

if(NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
endif()

set(SOURCES
    win32acrylic.manifest
    win32acrylic.rc
    runtimeobject.cpp
    acrylicapplication.h
    acrylicapplication_p.h
    acrylicapplication.cpp
    main.cpp
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

set(NTDDI_WIN10_19H1 0x0A000007)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    WIN32_LEAN_AND_MEAN
    WINRT_LEAN_AND_MEAN
    _CRT_SECURE_NO_WARNINGS
    UNICODE
    _UNICODE
    WINVER=${NTDDI_WIN10_19H1}
    _WIN32_WINNT=${NTDDI_WIN10_19H1}
    _ENABLE_EXTENDED_ALIGNED_STORAGE
)
unset(NTDDI_WIN10_19H1)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8 /await)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${PROJECT_NAME} PRIVATE /JMC)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE /guard:cf)
        target_link_options(${PROJECT_NAME} PRIVATE /GUARD:CF)
    endif()
endif()
