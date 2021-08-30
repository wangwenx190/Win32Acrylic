if(ACRYLICMANAGER_TARGET_ARCHITECTURE STREQUAL "x86")
    set(CMAKE_SYSTEM_PROCESSOR i686 CACHE STRING "" FORCE)
elseif(ACRYLICMANAGER_TARGET_ARCHITECTURE STREQUAL "x64")
    set(CMAKE_SYSTEM_PROCESSOR x86_64 CACHE STRING "" FORCE)
elseif(ACRYLICMANAGER_TARGET_ARCHITECTURE STREQUAL "arm")
    set(CMAKE_SYSTEM_PROCESSOR armv7 CACHE STRING "" FORCE)
elseif(ACRYLICMANAGER_TARGET_ARCHITECTURE STREQUAL "arm64")
    set(CMAKE_SYSTEM_PROCESSOR aarch64 CACHE STRING "" FORCE)
endif()

foreach(lang C CXX)
    set(CMAKE_${lang}_COMPILER_TARGET "${CMAKE_SYSTEM_PROCESSOR}-pc-windows-msvc" CACHE STRING "" FORCE)
endforeach()

find_program(CMAKE_ASM_COMPILER "clang-cl")
if(NOT CMAKE_ASM_COMPILER)
    find_program(CMAKE_ASM_COMPILER "cl")
endif()
find_program(CMAKE_C_COMPILER "clang-cl")
if(NOT CMAKE_C_COMPILER)
    find_program(CMAKE_C_COMPILER "cl")
endif()
find_program(CMAKE_CXX_COMPILER "clang-cl")
if(NOT CMAKE_CXX_COMPILER)
    find_program(CMAKE_CXX_COMPILER "cl")
endif()
#find_program(CMAKE_RC_COMPILER "llvm-rc")
if(NOT CMAKE_RC_COMPILER)
    find_program(CMAKE_RC_COMPILER "rc")
endif()
find_program(CMAKE_LINKER "lld-link")
if(NOT CMAKE_LINKER)
    find_program(CMAKE_LINKER "link")
endif()
find_program(CMAKE_AR "llvm-lib")
if(NOT CMAKE_AR)
    find_program(CMAKE_AR "lib")
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)

set(CMAKE_CXX_STANDARD_LIBRARIES "kernel32.lib" CACHE STRING "" FORCE)

# TODO: /await:Strict /permissive-
set(NTDDI_WIN10_19H1 0x0A000007)
set(_common_flags "/nologo /DWIN32 /D_WINDOWS /DWINVER=${NTDDI_WIN10_19H1} /D_WIN32_WINNT=${NTDDI_WIN10_19H1} /DUNICODE /D_UNICODE /DWIN32_LEAN_AND_MEAN /DWINRT_LEAN_AND_MEAN /DNOMINMAX /D_CRT_NON_CONFORMING_SWPRINTFS /D_CRT_SECURE_NO_WARNINGS /D_ENABLE_EXTENDED_ALIGNED_STORAGE /W4 /utf-8 /MP /await")
unset(NTDDI_WIN10_19H1)
set(CMAKE_C_FLAGS "${_common_flags} /std:c17" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "${_common_flags} /Zc:__cplusplus /std:c++20 /GR- /EHs-c-" CACHE STRING "" FORCE)
unset(_common_flags)
set(CMAKE_RC_FLAGS "-c65001 /DWIN32" CACHE STRING "" FORCE)

set(_debug_flags "/D_DEBUG /MTd /Zi /Ob0 /Od /RTC1 /JMC")
set(CMAKE_C_FLAGS_DEBUG "${_debug_flags}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "${_debug_flags}" CACHE STRING "" FORCE)
unset(_debug_flags)

set(_release_flags "/DNDEBUG /MT /O2 /Ob3 /Oi /Gy /GL /Zc:inline /guard:cf")
set(CMAKE_C_FLAGS_RELEASE "${_release_flags}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "${_release_flags}" CACHE STRING "" FORCE)
unset(_release_flags)

string(APPEND CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT " /nologo /DEBUG ")
string(APPEND CMAKE_SHARED_LINKER_FLAGS_DEBUG_INIT " /nologo /DEBUG ")
string(APPEND CMAKE_STATIC_LINKER_FLAGS_DEBUG_INIT " /nologo ")

set(_release_flags_shared_link "/nologo /DYNAMICBASE /NXCOMPAT /INCREMENTAL:NO /OPT:REF /OPT:ICF /LTCG /GUARD:CF")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${_release_flags_shared_link}" CACHE STRING "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${_release_flags_shared_link}" CACHE STRING "" FORCE)
unset(_release_flags_shared_link)
string(APPEND CMAKE_STATIC_LINKER_FLAGS_RELEASE_INIT " /nologo ")
