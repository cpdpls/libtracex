cmake_minimum_required(VERSION 4.0)

project(tx_tracer VERSION 1.0)

add_subdirectory(external/AbstractIO)

add_executable(tx_tracer
    main.c
    common/src/trace_errno.c
)

target_include_directories(tx_tracer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/common/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(tx_tracer
    PRIVATE
        abstractio
)