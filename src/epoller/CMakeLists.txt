add_library(epoller "")

target_sources(epoller
PRIVATE
    epoller.cpp
PUBLIC
    epoller.h
)

target_include_directories(epoller
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

