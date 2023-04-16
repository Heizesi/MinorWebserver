add_library(threadpool "")

target_sources(threadpool
PRIVATE
    threadpool.cpp
PUBLIC
    threadpool.h
)

target_include_directories(threadpool
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

