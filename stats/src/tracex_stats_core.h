#ifndef TRACEX_STATS_CORE_H
#define TRACEX_STATS_CORE_H

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "tracex_stats/tracex_stats_errno.h"

struct tracex_stats_handler {
    lua_State *lua_state;
    const char *lua_err_str;
};

#endif