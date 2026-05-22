cmake_minimum_required(VERSION 4.0)

project(tx_tracer VERSION 1.0)

add_executable(tx_tracer main.c)
target_include_directories(tx_tracer PUBLIC ${CMAKE_CURRENT_LIST_DIR}/common/inc)
target_link_libraries(tx_tracer tracelib)

add_library(tracelib STATIC)

target_sources( tracelib
                PRIVATE
                ${CMAKE_CURRENT_LIST_DIR}/common/src/trace_errno.c
                ${CMAKE_CURRENT_LIST_DIR}/src/trace_io/trace_io_file.c
                ${CMAKE_CURRENT_LIST_DIR}/src/trace_io/trace_io_tcp.c
                ${CMAKE_CURRENT_LIST_DIR}/src/trace_parser/trace_header.c

)
target_include_directories(tracelib
                            PRIVATE
                            ${CMAKE_CURRENT_LIST_DIR}/common/inc
                            ${CMAKE_CURRENT_LIST_DIR}/include/trace_io
                            ${CMAKE_CURRENT_LIST_DIR}/include/trace_parser
)