add_executable(webserver main.cpp)

add_subdirectory(buffer)
add_subdirectory(epoller)
add_subdirectory(http)
add_subdirectory(log)
add_subdirectory(server)
add_subdirectory(threadpool)
add_subdirectory(timerheap)

target_link_libraries(webserver
PRIVATE
    buffer
    epoller
    http
    log
    server
    threadpool
    timerheap
)