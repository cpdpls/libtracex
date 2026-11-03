#include "tracex_stats_scripting.h"
#include "tracex_stats/tracex_stats_errno.h"
#include "tracex_stats_core.h"

#include <stdlib.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>


static tracex_stats_ret_t check_function_exists(lua_State *lua_state, const char *function_name);
static tracex_stats_ret_t check_table_exists(lua_State *lua_state, const char *table_name);

tracex_stats_ret_t tracex_stats_scripting_load_script(tracex_stats_handler_t *handler, const char *script_path)
{
	tracex_stats_ret_t status;
	int lua_error;

	if ((lua_error = luaL_dofile(handler->lua_state, script_path))!= LUA_OK) {

		status = TRACEX_STATS_LUA_ERROR;
		handler->lua_err_str = lua_tostring(handler->lua_state, -1);
		lua_pop(handler->lua_state, 1);
		goto handle_exit;

	}

    if ((status = check_function_exists(handler->lua_state, "on_event")) != TRACEX_STATS_SUCCESS)
	    goto handle_exit;
    
    if ((status = check_function_exists(handler->lua_state, "get_result")) != TRACEX_STATS_SUCCESS)
	    goto handle_exit;

    if ((status = check_table_exists(handler->lua_state, "objects")) != TRACEX_STATS_SUCCESS)
	    goto handle_exit;

    if ((status = check_table_exists(handler->lua_state, "events")) != TRACEX_STATS_SUCCESS)
	    goto handle_exit;

    status = TRACEX_STATS_SUCCESS;
    goto handle_exit;
    
handle_exit:
	return status;
}

tracex_stats_ret_t tracex_stats_scripting_push_object(tracex_stats_handler_t *handler, const struct tracex_stats_object *object)
{
    lua_getglobal(handler->lua_state, "objects");
    lua_pushinteger(handler->lua_state, object->pointer);

    lua_newtable(handler->lua_state);
    lua_pushinteger(handler->lua_state, object->pointer);
    lua_setfield(handler->lua_state, -2, "ptr");

    lua_pushinteger(handler->lua_state, object->type);
    lua_setfield(handler->lua_state, -2, "type");

    lua_pushstring(handler->lua_state, object->name);
    lua_setfield(handler->lua_state, -2, "name");

    lua_pushinteger(handler->lua_state, object->param1);
    lua_setfield(handler->lua_state, -2, "param1");

    lua_pushinteger(handler->lua_state, object->param2);
    lua_setfield(handler->lua_state, -2, "param2");

    lua_settable(handler->lua_state, -3);
    lua_pop(handler->lua_state, 1); 
}
tracex_stats_ret_t tracex_stats_scripting_push_event(tracex_stats_handler_t *handler, const struct tracex_stats_event *event)
{
	tracex_stats_ret_t status;

    lua_getglobal(handler->lua_state, "on_event");


	lua_pushinteger(handler->lua_state, event->event_id);
	lua_pushinteger(handler->lua_state, event->thread_pointer);
	lua_pushinteger(handler->lua_state, event->thread_priority);
	lua_pushinteger(handler->lua_state, event->i1);
	lua_pushinteger(handler->lua_state, event->i2);
	lua_pushinteger(handler->lua_state, event->i3);
	lua_pushinteger(handler->lua_state, event->i4);
	lua_pushinteger(handler->lua_state, event->timestamp);

	if (lua_pcall(handler->lua_state, 8, 0, 0) != LUA_OK) {
		status = TRACEX_STATS_LUA_ERROR;
		handler->lua_err_str = lua_tostring(handler->lua_state, -1);
		lua_pop(handler->lua_state, 1);
		goto handle_exit;
	}

	status = TRACEX_STATS_SUCCESS;

handle_exit:
	return status;
}

static tracex_stats_ret_t lua_table_to_results(lua_State *L, tracex_stats_results **out)
{
    tracex_stats_results *res;
    size_t i, r, c;
    const char *t;

    if (!lua_istable(L, -1))
        return TRACEX_STATS_LUA_ERROR;

    res = (tracex_stats_results *)calloc(1, sizeof(*res));
    if (res == NULL)
        return TRACEX_STATS_ALLOCATION_FAILURE;

    lua_getfield(L, -1, "name");
    if (lua_isstring(L, -1))
        strncpy(res->name, lua_tostring(L, -1), sizeof(res->name) - 1);
    lua_pop(L, 1);

    lua_getfield(L, -1, "description");
    if (lua_isstring(L, -1))
        strncpy(res->description, lua_tostring(L, -1), sizeof(res->description) - 1);
    lua_pop(L, 1);

    lua_getfield(L, -1, "suggested_view");
    if (lua_isstring(L, -1))
        strncpy(res->suggested_view, lua_tostring(L, -1), sizeof(res->suggested_view) - 1);
    lua_pop(L, 1);

    /* columns */
    lua_getfield(L, -1, "columns");
    if (lua_istable(L, -1)) {
        res->n_columns = (size_t)lua_rawlen(L, -1);
        if (res->n_columns > 0) {
            res->columns = (struct tracex_stats_column *)calloc(res->n_columns,
                                                                sizeof(*res->columns));
            if (res->columns == NULL) {
                lua_pop(L, 1);
                free(res);
                return TRACEX_STATS_ALLOCATION_FAILURE;
            }

            for (i = 0; i < res->n_columns; i++) {
                lua_rawgeti(L, -1, (int)(i + 1));

                lua_getfield(L, -1, "name");
                if (lua_isstring(L, -1))
                    strncpy(res->columns[i].name, lua_tostring(L, -1),
                            sizeof(res->columns[i].name) - 1);
                lua_pop(L, 1);

                lua_getfield(L, -1, "type");
                t = lua_tostring(L, -1);
                if (t != NULL) {
                    if (strcmp(t, "uint32") == 0)
                        res->columns[i].type = COL_UINT32;
                    else if (strcmp(t, "uint64") == 0)
                        res->columns[i].type = COL_UINT64;
                    else if (strcmp(t, "double") == 0)
                        res->columns[i].type = COL_DOUBLE;
                    else if (strcmp(t, "string") == 0)
                        res->columns[i].type = COL_STRING;
                    else if (strcmp(t, "bool") == 0)
                        res->columns[i].type = COL_BOOL;
                }
                lua_pop(L, 1);

                lua_getfield(L, -1, "unit");
                if (lua_isstring(L, -1))
                    strncpy(res->columns[i].unit, lua_tostring(L, -1),
                            sizeof(res->columns[i].unit) - 1);
                lua_pop(L, 1);

                lua_getfield(L, -1, "description");
                if (lua_isstring(L, -1))
                    strncpy(res->columns[i].description, lua_tostring(L, -1),
                            sizeof(res->columns[i].description) - 1);
                lua_pop(L, 1);

                lua_pop(L, 1); /* column table */
            }
        }
    }
    lua_pop(L, 1); /* columns */

    /* rows */
    lua_getfield(L, -1, "rows");
    if (lua_istable(L, -1)) {
        res->n_rows = (size_t)lua_rawlen(L, -1);
        if (res->n_rows > 0) {
            res->rows = (struct tracex_stats_row *)calloc(res->n_rows, sizeof(*res->rows));
            if (res->rows == NULL) {
                lua_pop(L, 1);
                free(res->columns);
                free(res);
                return TRACEX_STATS_ALLOCATION_FAILURE;
            }

            for (r = 0; r < res->n_rows; r++) {
                lua_rawgeti(L, -1, (int)(r + 1));

                res->rows[r].values = calloc(res->n_columns, sizeof(*res->rows[r].values));
                if (res->rows[r].values == NULL) {
                    lua_pop(L, 2);
                    /* caller should free partial via results_free pattern;
                     * simplify: free what we have */
                    res->n_rows = r;
                    *out = res;
                    return TRACEX_STATS_ALLOCATION_FAILURE;
                }

                for (c = 0; c < res->n_columns; c++) {
                    lua_rawgeti(L, -1, (int)(c + 1));

                    switch (res->columns[c].type) {
                    case COL_UINT32:
                        res->rows[r].values[c].u32 = (uint32_t)lua_tointeger(L, -1);
                        break;
                    case COL_UINT64:
                        res->rows[r].values[c].u64 = (uint64_t)lua_tointeger(L, -1);
                        break;
                    case COL_DOUBLE:
                        res->rows[r].values[c].f64 = lua_tonumber(L, -1);
                        break;
                    case COL_STRING: {
                        const char *s = lua_tostring(L, -1);
                        res->rows[r].values[c].str = strdup(s != NULL ? s : "");
                        break;
                    }
                    case COL_BOOL:
                        res->rows[r].values[c].b = lua_toboolean(L, -1);
                        break;
                    }

                    lua_pop(L, 1); /* cell */
                }

                lua_pop(L, 1); /* row */
            }
        }
    }
    lua_pop(L, 1); /* rows */

    *out = res;
    return TRACEX_STATS_SUCCESS;
}

tracex_stats_ret_t tracex_stats_scripting_get_results(tracex_stats_handler_t *handler,
                                                      tracex_stats_results **results)
{
    tracex_stats_ret_t status;
    lua_State *L = handler->lua_state;

    *results = NULL;

    lua_getglobal(L, "get_result");
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        return TRACEX_STATS_SCRIPT_MISSING_FUNCTION;
    }

    if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
        handler->lua_err_str = lua_tostring(L, -1);
        lua_pop(L, 1);
        return TRACEX_STATS_LUA_ERROR;
    }

    /* stack: [ result_table ] */
    status = lua_table_to_results(L, results);
    lua_pop(L, 1); /* result table */

    if (status != TRACEX_STATS_SUCCESS && *results != NULL) {
        /* optional: free partial — or leave to caller */
    }

    return status;
}


static tracex_stats_ret_t check_function_exists(lua_State *lua_state, const char *function_name)
{
	tracex_stats_ret_t status;

	lua_getglobal(lua_state, function_name);
	if (!lua_isfunction(lua_state, -1)) {
		status = TRACEX_STATS_SCRIPT_MISSING_FUNCTION;
		goto handle_exit;
	}

	status = TRACEX_STATS_SUCCESS;
    
handle_exit:
    lua_pop(lua_state, 1);
	return status;
}

static tracex_stats_ret_t check_table_exists(lua_State *lua_state, const char *table_name)
{
    tracex_stats_ret_t status;

    lua_getglobal(lua_state, table_name);
    if (!lua_istable(lua_state, -1)) {
        status =  TRACEX_STATS_SCRIPT_MISSING_TABLE;
        goto handle_exit;
    }

    status =  TRACEX_STATS_SUCCESS;

handle_exit:
    lua_pop(lua_state, 1);
	return status;
}
