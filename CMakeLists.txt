cmake_minimum_required(VERSION 3.21.2 FATAL_ERROR)

project(Win32AcrylicHelper LANGUAGES CXX RC)

set(ACRYLIC_TARGET_ARCHITECTURE "x64")

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
    WindowsVersion.h
    WindowsVersion.cpp
    main.cpp
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES})
