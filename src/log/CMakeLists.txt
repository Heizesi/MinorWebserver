add_library(log "")

target_sources(log
PRIVATE
    log.cpp
PUBLIC
    log.h
    blockqueue.h
)

target_include_directories(log
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(log
PRIVATE
    buffer
)