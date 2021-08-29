enable_language(RC)

set(SOURCES
    demo.manifest
    demo.rc
    acrylicmanager.hpp
    main.cpp
)

add_executable(Demo WIN32 ${SOURCES})
