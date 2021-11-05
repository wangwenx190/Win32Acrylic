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

cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(Win32AcrylicHelper VERSION 1.0.0.0 LANGUAGES CXX RC)

# Defaults to release builds if not set by the user.
if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

# Enable link time optimization for release builds.
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON CACHE BOOL "" FORCE)

# Request C++20.
set(CMAKE_CXX_STANDARD "20" CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "" FORCE)
set(CMAKE_CXX_EXTENSIONS OFF CACHE BOOL "" FORCE)

# Don't link to any libraries by default.
set(CMAKE_C_STANDARD_LIBRARIES "" CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD_LIBRARIES "" CACHE STRING "" FORCE)

# To make use of VC-LTL we have to use "/MT(d)" unconditionally.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)

# Remove parameters that disable exception handling. WinRT needs it.
string(REGEX REPLACE "[-|/]EHs-c-" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
# Disable runtime type information (RTTI) generation. We don't need it.
string(REGEX REPLACE "[-|/]GR" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
# We use "/O1 /Ob1" for release builds, so remove related parameters first.
string(REGEX REPLACE "[-|/]O[d|1|2|3|i]" "" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
string(REGEX REPLACE "[-|/]Ob[0|1|2|3]" "" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})

# Change code page to UTF-8 (65001) and suppress the copyright messages.
string(APPEND CMAKE_RC_FLAGS " -c65001 /nologo ")
string(APPEND CMAKE_CXX_FLAGS " /await:strict /bigobj /EHsc /d2FH4 /GR- /MP /utf-8 /W4 /WX /permissive- /Zc:char8_t,__cplusplus,externConstexpr,hiddenFriend,lambda,referenceBinding,rvalueCast,strictStrings,ternary,throwingNew,trigraphs ")
string(APPEND CMAKE_CXX_FLAGS_RELEASE " /O1 /Ob1 /guard:cf /guard:ehcont /Gw /Gy /Zc:inline ")
string(APPEND CMAKE_EXE_LINKER_FLAGS_RELEASE " /CETCOMPAT /GUARD:CF /GUARD:EHCONT /HIGHENTROPYVA /LARGEADDRESSAWARE /OPT:REF /OPT:ICF /WX ")

# Include VC-LTL helper script.
include(VC-LTL.cmake)

# Win32AcrylicHelper
set(SOURCES_Win32AcrylicHelper
    Win32AcrylicHelper/Resource.h
    Win32AcrylicHelper/Definitions.h
    Win32AcrylicHelper/pch.h Win32AcrylicHelper/pch.cpp
    Win32AcrylicHelper/Color.hpp
    Win32AcrylicHelper/VersionNumber.hpp
    Win32AcrylicHelper/OperationResult.h Win32AcrylicHelper/OperationResult.cpp
    Win32AcrylicHelper/WindowsVersion.h Win32AcrylicHelper/WindowsVersion.cpp
    Win32AcrylicHelper/Utils.h Win32AcrylicHelper/Utils.cpp
    Win32AcrylicHelper/Window.h Win32AcrylicHelper/Window.cpp
    Win32AcrylicHelper/Thunks/SystemLibrary.h Win32AcrylicHelper/Thunks/SystemLibrary.cpp
    Win32AcrylicHelper/Thunks/SystemLibraryManager.h Win32AcrylicHelper/Thunks/SystemLibraryManager.cpp
    Win32AcrylicHelper/Thunks/WindowsAPIThunks.h Win32AcrylicHelper/Thunks/WindowsAPIThunks.cpp
    Win32AcrylicHelper/Thunks/ComBase_Thunk.cpp Win32AcrylicHelper/Thunks/User32_Thunk.cpp
    Win32AcrylicHelper/Thunks/Gdi32_Thunk.cpp Win32AcrylicHelper/Thunks/UxTheme_Thunk.cpp
    Win32AcrylicHelper/Thunks/AdvApi32_Thunk.cpp Win32AcrylicHelper/Thunks/Ole32_Thunk.cpp
    Win32AcrylicHelper/Thunks/DwmApi_Thunk.cpp Win32AcrylicHelper/Thunks/Shell32_Thunk.cpp
    Win32AcrylicHelper/Thunks/OleAut32_Thunk.cpp Win32AcrylicHelper/Thunks/DispatcherQueue_Thunk.h
    Win32AcrylicHelper/Thunks/CoreMessaging_Thunk.cpp Win32AcrylicHelper/Thunks/WinRTWrappers.cpp
    Win32AcrylicHelper/Thunks/Undocumented.h Win32AcrylicHelper/Thunks/Undocumented.cpp
)
add_library(${PROJECT_NAME} STATIC ${SOURCES_Win32AcrylicHelper})
add_library(wangwenx190::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC
    Win32AcrylicHelper
    Win32AcrylicHelper/Thunks
)
target_link_libraries(${PROJECT_NAME} PUBLIC
    Kernel32.lib
)
set(_WIN32_WINNT_WIN10 0x0A00)
set(NTDDI_WIN10_CO 0x0A00000B)
target_compile_definitions(${PROJECT_NAME} PUBLIC
    _CRT_NON_CONFORMING_SWPRINTFS _CRT_SECURE_NO_WARNINGS
    _ENABLE_EXTENDED_ALIGNED_STORAGE
    NOMINMAX
    UNICODE _UNICODE
    WIN32_LEAN_AND_MEAN WINRT_LEAN_AND_MEAN
    WINVER=${_WIN32_WINNT_WIN10} _WIN32_WINNT=${_WIN32_WINNT_WIN10}
    _WIN32_IE=${_WIN32_WINNT_WIN10} NTDDI_VERSION=${NTDDI_WIN10_CO}
    _KERNEL32_ _USER32_ _SHELL32_ _GDI32_ _OLE32_ _OLEAUT32_
    _ADVAPI32_ _COMBASEAPI_ _DWMAPI_ _UXTHEME_ _ROAPI_
)
unset(SOURCES_Win32AcrylicHelper)
unset(_WIN32_WINNT_WIN10)
unset(NTDDI_WIN10_CO)

# Demo applications
set(SOURCES_Xaml
    Xaml/XamlWindow.h Xaml/XamlWindow.cpp
    Xaml/XamlApplication.h Xaml/XamlApplication.cpp
    Xaml/main_Xaml.cpp
)

set(SOURCES_Composition
    Composition/CompositionWindow.h Composition/CompositionWindow.cpp
    Composition/CompositionApplication.h Composition/CompositionApplication.cpp
    Composition/main_Composition.cpp
)

set(_target_arch "32")
if("x${CMAKE_SIZEOF_VOID_P}" STREQUAL "x8")
    set(_target_arch "64")
endif()
set(_target_filename_suffix ${CMAKE_BUILD_TYPE}${_target_arch})

set(_demo_types Xaml Composition)
foreach(_type IN LISTS _demo_types)
    set(_current_subproject_name Demo_${_type})
    add_executable(${_current_subproject_name} WIN32
        Win32AcrylicHelper/Win32AcrylicHelper.rc Win32AcrylicHelper/Win32AcrylicHelper.manifest
        ${SOURCES_${_type}}
    )
    target_link_libraries(${_current_subproject_name} PRIVATE
        wangwenx190::${PROJECT_NAME}
    )
    set_target_properties(${_current_subproject_name} PROPERTIES
        OUTPUT_NAME ${_current_subproject_name}${_target_filename_suffix}
    )
    unset(_current_subproject_name)
endforeach()

unset(SOURCES_Xaml)
unset(SOURCES_Composition)
unset(_demo_types)
unset(_target_filename_suffix)
unset(_target_arch)
