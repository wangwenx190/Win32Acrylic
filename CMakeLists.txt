cmake_minimum_required(VERSION 3.18)

project(Win32Acrylic LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES
    app.manifest # XAML Islands applications must have a manifest file which contains "maxversiontested"
    main.cpp
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

target_compile_definitions(${PROJECT_NAME} PRIVATE
    WIN32_LEAN_AND_MEAN
    _CRT_SECURE_NO_WARNINGS
    UNICODE
    _UNICODE
)

# WinRT library: RuntimeObject.lib
target_link_libraries(${PROJECT_NAME} PRIVATE RuntimeObject)
