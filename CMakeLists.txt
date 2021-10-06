#[[
  MIT License

  Copyright (C) 2021 by wangwenx190 (Yuhang Zhao)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
]]

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

include("${PROJECT_SOURCE_DIR}/VC-LTL.cmake")

set(SOURCES
    Win32AcrylicHelper.rc Win32AcrylicHelper.manifest
    Resource.h
    Definitions.h
    pch.h pch.cpp
    Color.hpp
    VersionNumber.hpp
    RuntimeObject.cpp
    OperationResult.h OperationResult.cpp
    WindowsVersion.h WindowsVersion.cpp
    Utils.h Utils.cpp
    SystemLibrary.h SystemLibrary.cpp
    SystemLibraryManager.h SystemLibraryManager.cpp
    Window.h Window.cpp
    XamlWindow.h XamlWindow.cpp
    XamlApplication.h XamlApplication.cpp
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
