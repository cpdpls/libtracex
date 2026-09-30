cmake_minimum_required(VERSION 3.16...4.0)
project(libtracex VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Force cJSON to build as a static library and skip its tests
set(CJSON_OVERRIDE_BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
set(CJSON_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ENABLE_SANITIZERS OFF CACHE BOOL "" FORCE)
set(ENABLE_CJSON_TEST OFF CACHE BOOL "Enable building cJSON test" FORCE)

add_subdirectory(external/cjson)

add_library(tracex STATIC
    src/tracex_errno.c
    src/tracex_header.c
    src/tracex_core.c
    src/tracex_obj.c
    src/tracex_event.c
    src/tracex_debug.c
    src/tracex_utils.c
    src/tracex_obj_str.c
    src/tracex_event_str.c
)

target_include_directories(tracex
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/external/cjson
        ${CMAKE_CURRENT_SOURCE_DIR}/external/uthash/include
)

target_link_libraries(tracex PRIVATE cjson)

option(BUILD_TRACEX_TEST "Build the tracex test executable (tests/main.c)" OFF)

if(BUILD_TRACEX_TEST)
    add_executable(tracex_test
        tests/main.c
    )
    target_link_libraries(tracex_test PRIVATE tracex)
endif()