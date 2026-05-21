cmake_minimum_required(VERSION 4.0)

project(tx_tracer VERSION 1.0)

add_subdirectory(trace_parser)
add_executable(tx_tracer main.c)
target_include_directories(tx_tracer PUBLIC ${CMAKE_CURRENT_LIST_DIR}/trace_parser/inc)
target_link_libraries(tx_tracer traceparser)

