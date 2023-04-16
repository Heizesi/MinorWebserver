add_library(buffer "")

target_sources(buffer
PRIVATE
    buffer.cpp
PUBLIC
    buffer.h
)

target_include_directories(buffer
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

