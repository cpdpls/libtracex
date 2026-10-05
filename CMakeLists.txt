cmake_minimum_required(VERSION 3.16...4.0)
project(libtracex VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")
# Options used to turn off the building of certain libraries and tests
option(TRACEX_BUILD_LABELS   "Build the labels library"           ON)
option(TRACEX_BUILD_STATS    "Build the statistics / Lua library" OFF)
option(TRACEX_BUILD_EXAMPLES "Build example programs"             ON)


# Force cJSON to build statically and skip the building of the tests
set(CJSON_OVERRIDE_BUILD_SHARED_LIBS ON  CACHE BOOL "" FORCE)
set(CJSON_BUILD_SHARED_LIBS          OFF CACHE BOOL "" FORCE)
set(ENABLE_SANITIZERS                OFF CACHE BOOL "" FORCE)
set(ENABLE_CJSON_TEST                OFF CACHE BOOL "Enable building cJSON test" FORCE)

# Always build the core library in any cases 
add_subdirectory(core)

# Build the labels library if enabled
if(TRACEX_BUILD_LABELS)
    add_subdirectory(external/cjson)
    add_subdirectory(resolver)
endif()

# Build the statistics library if enabled
if(TRACEX_BUILD_STATS)
    add_subdirectory(stats)
endif()

# Build the testing program if enabled
if(TRACEX_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
