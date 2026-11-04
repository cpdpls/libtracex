cmake_minimum_required(VERSION 3.16...4.0)

project(libtracex VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# Standalone vs subdirectory
# ---------------------------------------------------------------------------
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(TRACEX_IS_TOP_LEVEL ON)
else()
    set(TRACEX_IS_TOP_LEVEL OFF)
endif()

option(TRACEX_BUILD_LABELS   "Build the labels library"           ON)
option(TRACEX_BUILD_STATS    "Build the statistics / Lua library" OFF)
option(TRACEX_BUILD_EXAMPLES "Build example programs"             ${TRACEX_IS_TOP_LEVEL})

# Examples need labels + stats
if(TRACEX_BUILD_EXAMPLES)
    if(NOT TRACEX_BUILD_LABELS)
        message(STATUS "TRACEX_BUILD_EXAMPLES is ON: enabling TRACEX_BUILD_LABELS")
        set(TRACEX_BUILD_LABELS ON CACHE BOOL "Build the labels library" FORCE)
    endif()
    if(NOT TRACEX_BUILD_STATS)
        message(STATUS "TRACEX_BUILD_EXAMPLES is ON: enabling TRACEX_BUILD_STATS")
        set(TRACEX_BUILD_STATS ON CACHE BOOL "Build the statistics / Lua library" FORCE)
    endif()
endif()

# cJSON only when labels are built
if(TRACEX_BUILD_LABELS)
    set(CJSON_OVERRIDE_BUILD_SHARED_LIBS ON  CACHE BOOL "" FORCE)
    set(CJSON_BUILD_SHARED_LIBS          OFF CACHE BOOL "" FORCE)
    set(ENABLE_SANITIZERS                OFF CACHE BOOL "" FORCE)
    set(ENABLE_CJSON_TEST                OFF CACHE BOOL "Enable building cJSON test" FORCE)
endif()

# ---------------------------------------------------------------------------
# Components
# ---------------------------------------------------------------------------
add_subdirectory(core)

if(TRACEX_BUILD_LABELS)
    add_subdirectory(external/cjson)
    add_subdirectory(labels)
endif()

if(TRACEX_BUILD_STATS)
    add_subdirectory(stats)
endif()

if(TRACEX_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ---------------------------------------------------------------------------
# Single consumer-facing target: link only "tracex"
# ---------------------------------------------------------------------------
add_library(tracex INTERFACE)
add_library(tracex::tracex ALIAS tracex)

target_link_libraries(tracex INTERFACE
    tracex_core
)

if(TARGET labels_engine)
    target_link_libraries(tracex INTERFACE labels_engine)
endif()

if(TARGET tracex_stats)
    target_link_libraries(tracex INTERFACE tracex_stats)
endif()

# Optional: nice message when configuring
message(STATUS "libtracex: labels=${TRACEX_BUILD_LABELS} stats=${TRACEX_BUILD_STATS} examples=${TRACEX_BUILD_EXAMPLES}")