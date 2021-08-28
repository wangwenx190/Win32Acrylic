enable_language(RC)

set(SOURCES
    demo.manifest
    demo.rc
    main.cpp
)

add_executable(Demo WIN32 ${SOURCES})

target_link_libraries(Demo PRIVATE
    wangwenx190::AcrylicManager
)
