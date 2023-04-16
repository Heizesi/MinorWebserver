add_library(server "")

target_sources(server
PRIVATE
    webserver.cpp
PUBLIC
    webserver.h
)

target_include_directories(server
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(server
PRIVATE
    buffer
    epoller
    http
    log
    threadpool
    timerheap
)