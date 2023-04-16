add_library(http "")

target_sources(http
PRIVATE
    httprequest.cpp
    httpresponse.cpp
    httpconn.cpp
PUBLIC
    httprequest.h
    httpresponse.h
    httpconn.h
)

target_include_directories(http
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(http
PRIVATE
    buffer
    log
)