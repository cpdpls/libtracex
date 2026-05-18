cmake_minimum_required(VERSION 3.10)

project(tx_tracer VERSION 1.0)

add_executable(tx_tracer main.c)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/trace_parser)
target_include_directories(tx_tracer PUBLIC ${CMAKE_CURRENT_LIST_DIR}/trace_parser/inc)

