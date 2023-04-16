add_library(timerheap "")

target_sources(timerheap
PRIVATE
    timerheap.cpp
PUBLIC
    timerheap.h
)

target_include_directories(timerheap
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

