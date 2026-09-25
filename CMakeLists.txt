cmake_minimum_required(VERSION 4.0)

project(tx_tracer VERSION 1.0)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(ENABLE_CJSON_TEST OFF CACHE BOOL "Enable building cJSON test" FORCE)
add_compile_definitions(ENABLE_CJSON_TEST=Off)

add_subdirectory(external/cjson)

add_executable(tx_tracer
    main.c
    src/tracex_errno.c
    src/tracex_header.c
    src/tracex_core.c
    src/tracex_obj.c
    src/tracex_event.c
    src/tracex_debug.c
    src/tracex_utils.c
    src/tracex_obj_str.c
)

target_include_directories(tx_tracer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/external/cjson
)

target_link_libraries(tx_tracer
    PRIVATE
    cjson
)