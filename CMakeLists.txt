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

if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

if(NOT DEFINED CMAKE_DEBUG_POSTFIX)
    set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "" FORCE)
endif()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON CACHE BOOL "" FORCE)

set(CMAKE_CXX_STANDARD "20" CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "" FORCE)
set(CMAKE_CXX_EXTENSIONS OFF CACHE BOOL "" FORCE)

# Don't link to any libraries by default.
set(CMAKE_C_STANDARD_LIBRARIES "" CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD_LIBRARIES "" CACHE STRING "" FORCE)

# To make use of VC-LTL we have to use "/MT(d)" unconditionally.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)

string(REGEX REPLACE "[-|/]EHs-c-" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
string(REGEX REPLACE "[-|/]GR" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
string(REGEX REPLACE "[-|/]O[d|1|2|3|i]" "" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
string(REGEX REPLACE "[-|/]Ob[0|1|2|3]" "" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})

string(APPEND CMAKE_RC_FLAGS " -c65001 /nologo ")
string(APPEND CMAKE_CXX_FLAGS " /await:strict /bigobj /EHsc /GR- /MP /utf-8 /W4 /WX /permissive- /Zc:char8_t,__cplusplus,externConstexpr,hiddenFriend,lambda,referenceBinding,rvalueCast,strictStrings,ternary,throwingNew,trigraphs ")
string(APPEND CMAKE_CXX_FLAGS_RELEASE " /O1 /Ob1 /Gw /Gy /Zc:inline ")
string(APPEND CMAKE_EXE_LINKER_FLAGS_RELEASE " /HIGHENTROPYVA /LARGEADDRESSAWARE /OPT:REF /OPT:ICF /WX ")

include(VC-LTL.cmake)

set(SOURCES_COMMON
    Win32AcrylicHelper.rc Win32AcrylicHelper.manifest
    Resource.h
    Definitions.h
    pch.h pch.cpp
    Color.hpp
    VersionNumber.hpp
    OperationResult.h OperationResult.cpp
    WindowsVersion.h WindowsVersion.cpp
    Utils.h Utils.cpp
    SystemLibrary.h SystemLibrary.cpp
    SystemLibraryManager.h SystemLibraryManager.cpp
    WindowsAPIThunks.h WindowsAPIThunks.cpp
    RuntimeObject_Thunk.cpp User32_Thunk.cpp Gdi32_Thunk.cpp
    UxTheme_Thunk.cpp AdvApi32_Thunk.cpp Ole32_Thunk.cpp
    DwmApi_Thunk.cpp Shell32_Thunk.cpp
    Window.h Window.cpp
)

set(SOURCES_Xaml
    ${SOURCES_COMMON}
    XamlWindow.h XamlWindow.cpp
    XamlApplication.h XamlApplication.cpp
    main_Xaml.cpp
)

set(SOURCES_Composition
    ${SOURCES_COMMON}
    CompositionWindow.h CompositionWindow.cpp
    CompositionApplication.h CompositionApplication.cpp
    main_Composition.cpp
)

set(_WIN32_WINNT_WIN10 0x0A00)
set(NTDDI_WIN10_CO 0x0A00000B)
set(_target_extra_definitions
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

set(_target_arch_suffix "32")
if("x${CMAKE_SIZEOF_VOID_P}" STREQUAL "x8")
    set(_target_arch_suffix "64")
endif()
set(_target_full_suffix ${CMAKE_BUILD_TYPE}${_target_arch_suffix})

set(_demo_types Xaml Composition)
foreach(_type IN LISTS _demo_types)
    set(_current_subproject_name Demo_${_type})
    add_executable(${_current_subproject_name} WIN32
        ${SOURCES_${_type}}
    )
    target_compile_definitions(${_current_subproject_name} PRIVATE
        ${_target_extra_definitions}
    )
    target_link_libraries(${_current_subproject_name} PRIVATE
        Kernel32.lib
    )
    set_target_properties(${_current_subproject_name} PROPERTIES
        OUTPUT_NAME ${_current_subproject_name}${_target_full_suffix}
    )
endforeach()
