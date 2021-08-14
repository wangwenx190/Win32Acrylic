cmake_minimum_required(VERSION 3.21.1 FATAL_ERROR)

project(AcrylicApplication VERSION 1.0.0.0 LANGUAGES CXX RC)

set(BUILD_SHARED_LIBS ON CACHE BOOL "")

set(CMAKE_CXX_STANDARD 17 CACHE STRING "") # TODO: find out how to support C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "")
set(CMAKE_CXX_EXTENSIONS OFF CACHE BOOL "")

if(NOT DEFINED CMAKE_DEBUG_POSTFIX)
    if(WIN32)
        set(CMAKE_DEBUG_POSTFIX d CACHE STRING "")
    else()
        set(CMAKE_DEBUG_POSTFIX _debug CACHE STRING "")
    endif()
endif()

if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "")
endif()

if(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug") AND BUILD_SHARED_LIBS)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON CACHE BOOL "")
endif()

if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE PATH "")
endif()

if(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib CACHE PATH "")
endif()

if(NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib CACHE PATH "")
endif()

set(SOURCES_LIB
    acrylicmanager_global.h
    runtimeobject.cpp
    acrylicmanager.h
    acrylicmanager_p.h
    acrylicmanager.cpp
)

if(BUILD_SHARED_LIBS)
    enable_language(RC)
    list(APPEND SOURCES acrylicmanager.rc)
endif()

add_library(AcrylicManager ${SOURCES_LIB})
add_library(wangwenx190::AcrylicManager ALIAS AcrylicManager)

set(NTDDI_WIN10_19H1 0x0A000007)
target_compile_definitions(AcrylicManager PRIVATE
    WIN32_LEAN_AND_MEAN
    WINRT_LEAN_AND_MEAN
    _CRT_SECURE_NO_WARNINGS
    UNICODE
    _UNICODE
    WINVER=${NTDDI_WIN10_19H1}
    _WIN32_WINNT=${NTDDI_WIN10_19H1}
    _ENABLE_EXTENDED_ALIGNED_STORAGE
    NOMINMAX
    ACRYLICMANAGER_BUILD_LIBRARY
)
unset(NTDDI_WIN10_19H1)

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(AcrylicManager PRIVATE
        ACRYLICMANAGER_STATIC
    )
endif()

if(MSVC)
    target_compile_options(AcrylicManager PRIVATE /utf-8 /await) # TODO: /permissive-
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(AcrylicManager PRIVATE /JMC)
    else()
        target_compile_options(AcrylicManager PRIVATE /guard:cf)
        target_link_options(AcrylicManager PRIVATE /GUARD:CF)
    endif()
endif()

set(SOURCES_APP
    acrylicapplication.manifest
    acrylicapplication.rc
    main.cpp
)

add_executable(Demo WIN32 ${SOURCES_APP})

target_link_libraries(Demo PRIVATE
    wangwenx190::AcrylicManager
)
