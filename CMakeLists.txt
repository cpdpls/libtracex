cmake_minimum_required(VERSION 4.0)

project(tx_tracer VERSION 1.0)

set(CMAKE_C_FLAGS "-O3 -Wall -Wextra")
set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(tx_tracer
    main.c
    src/tracex_errno.c
    src/tracex_header.c
    src/tracex_core.c
    src/tracex_obj.c
    src/tracex_event.c
    src/tracex_debug.c
    src/tracex_utils.c
)

target_include_directories(tx_tracer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)