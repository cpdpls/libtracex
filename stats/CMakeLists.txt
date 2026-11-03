# stats/CMakeLists.txt
# Optional statistics engine (Lua). Does not depend on tracex_core.

find_package(Lua 5.1 REQUIRED)

add_library(tracex_stats STATIC
    src/tracex_stats_errno.c
    src/tracex_stats_core.c
    src/tracex_stats_scripting.c
)

add_library(tracex::stats ALIAS tracex_stats)

target_include_directories(tracex_stats
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${LUA_INCLUDE_DIR}
)

target_link_libraries(tracex_stats
    PRIVATE
        ${LUA_LIBRARIES}
)

message(STATUS "tracex_stats: Lua ${LUA_VERSION_STRING}")
message(STATUS "  include: ${LUA_INCLUDE_DIR}")
message(STATUS "  libs:    ${LUA_LIBRARIES}")