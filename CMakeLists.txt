cmake_minimum_required(VERSION 4.0)

project(tx_tracer VERSION 1.0)

add_executable(tx_tracer
    main.c
    src/tracex_errno.c
    src/tracex_header_int.c
    src/tracex_core.c
    src/tracex_obj_int.c
    src/tracex_event_int.c
)

target_include_directories(tx_tracer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/common/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)