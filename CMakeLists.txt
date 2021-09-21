cmake_minimum_required(VERSION 3.21.2 FATAL_ERROR)

project(Win32AcrylicHelper VERSION 1.0.0.0 LANGUAGES CXX RC)

if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

if(NOT DEFINED ACRYLIC_TARGET_ARCHITECTURE)
    if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
        set(ACRYLIC_TARGET_ARCHITECTURE "x64")
    else()
        set(ACRYLIC_TARGET_ARCHITECTURE "x86")
    endif()
endif()

include("${PROJECT_SOURCE_DIR}/toolchain.cmake")

set(SOURCES
    Win32AcrylicHelper.rc
    Win32AcrylicHelper.manifest
    Resource.h
    RuntimeObject.cpp
    SystemLibrary.h
    SystemLibrary.cpp
    SystemLibraryManager.h
    SystemLibraryManager.cpp
    WindowsVersion.hpp
    Utils.h
    Utils.cpp
    AcrylicApplication.h
    AcrylicApplication.cpp
    main.cpp
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

set(_target_arch_suffix "32")
if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    set(_target_arch_suffix "64")
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}${CMAKE_BUILD_TYPE}${_target_arch_suffix}
)
unset(_target_arch_suffix)
