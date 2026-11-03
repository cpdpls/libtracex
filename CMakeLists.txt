cmake_minimum_required(VERSION 3.16...4.0)
project(libtracex VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(TRACEX_BUILD_LABELS   "Build the labels library"           ON)
option(TRACEX_BUILD_STATS    "Build the statistics / Lua library" OFF)
option(TRACEX_BUILD_EXAMPLES "Build example programs"             ON)

# Examples need labels (→ cJSON) and stats (→ Lua)
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

# cJSON is only configured when labels are built
if(TRACEX_BUILD_LABELS)
    set(CJSON_OVERRIDE_BUILD_SHARED_LIBS ON  CACHE BOOL "" FORCE)
    set(CJSON_BUILD_SHARED_LIBS          OFF CACHE BOOL "" FORCE)
    set(ENABLE_SANITIZERS                OFF CACHE BOOL "" FORCE)
    set(ENABLE_CJSON_TEST                OFF CACHE BOOL "Enable building cJSON test" FORCE)
endif()

# Core is always built
add_subdirectory(core)

# Labels + cJSON only if requested
if(TRACEX_BUILD_LABELS)
    add_subdirectory(external/cjson)   # only needed for labels
    add_subdirectory(labels)
endif()

# Stats (Lua) only if requested
if(TRACEX_BUILD_STATS)
    add_subdirectory(stats)
endif()

# Examples (require labels + stats; enforced above)
if(TRACEX_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()