
add_library(tracex_core STATIC
    src/tracex_errno.c
    src/tracex_header.c
    src/tracex_core.c
    src/tracex_obj.c
    src/tracex_event.c
    src/tracex_endian.c
    src/tracex_debug.c
)

add_library(tracex::core ALIAS tracex_core)

target_include_directories(tracex_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
