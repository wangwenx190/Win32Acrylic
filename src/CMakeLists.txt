set(CMAKE_CXX_STANDARD_LIBRARIES "kernel32.lib" CACHE STRING "" FORCE)

set(SOURCES
    acrylicmanager_global.h
    runtimeobject.cpp
    dynamicapis.cpp
    acrylicmanager.h
    acrylicmanager_p.h
    acrylicmanager.cpp
)

if(BUILD_SHARED_LIBS)
    enable_language(RC)
    configure_file(acrylicmanager.rc.in ${PROJECT_BINARY_DIR}/acrylicmanager.rc @ONLY)
    list(APPEND SOURCES ${PROJECT_BINARY_DIR}/acrylicmanager.rc)
endif()

add_library(${PROJECT_NAME} ${SOURCES})
add_library(wangwenx190::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
)

set(NTDDI_WIN10_19H1 0x0A000007)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    WIN32_LEAN_AND_MEAN
    WINRT_LEAN_AND_MEAN
    UNICODE
    _UNICODE
    WINVER=${NTDDI_WIN10_19H1}
    _WIN32_WINNT=${NTDDI_WIN10_19H1}
    _ENABLE_EXTENDED_ALIGNED_STORAGE
    NOMINMAX
    _CRT_NON_CONFORMING_SWPRINTFS
    _CRT_SECURE_NO_WARNINGS
    ACRYLICMANAGER_BUILD_LIBRARY
    ACRYLICMANAGER_VERSION_STR=L\"${PROJECT_VERSION}\"
)
unset(NTDDI_WIN10_19H1)

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        ACRYLICMANAGER_STATIC
    )
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8 /await) # TODO: /await:strict /permissive-
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${PROJECT_NAME} PRIVATE /JMC)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE /guard:cf)
        target_link_options(${PROJECT_NAME} PRIVATE /GUARD:CF)
    endif()
endif()
