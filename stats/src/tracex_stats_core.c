#include <stdlib.h>
#include <string.h>

#include "tracex_stats/tracex_stats.h"
#include "tracex_stats/tracex_stats_errno.h"
#include "tracex_stats_core.h"
#include "tracex_stats_scripting.h"

static tracex_stats_ret_t check_handle_valid(tracex_stats_handler_t *handler);

tracex_stats_ret_t tracex_stats_create_new_handler(tracex_stats_handler_t **new_handler)
{
    tracex_stats_ret_t status;
    struct tracex_stats_handler *tmp_handler;

	if (new_handler == NULL) {
		status = TRACEX_STATS_BAD_INPUT_PTR;
		goto handle_exit;
	}

	tmp_handler = (struct tracex_stats_handler *)malloc(sizeof(struct tracex_stats_handler));

    if (tmp_handler == NULL) {
	    status = TRACEX_STATS_ALLOCATION_FAILURE;
	    goto handle_exit;
    }

    tmp_handler->lua_state = luaL_newstate();
    if (tmp_handler->lua_state == NULL) {
	    status = TRACEX_STATS_LUA_ERROR;
	    goto handle_error;
    }

    luaL_openlibs(tmp_handler->lua_state);

    status = TRACEX_STATS_SUCCESS;
    goto handle_exit;

handle_error:
	tracex_stats_destroy_handler(&tmp_handler);

handle_exit:
    if (new_handler != NULL)
	    *new_handler = tmp_handler;
    return status;
}

void tracex_stats_destroy_handler(tracex_stats_handler_t **handler)
{
    if (handler != NULL && *handler != NULL) {
        if ((*handler)->lua_state != NULL) {
            lua_close((*handler)->lua_state);
        }

	memset(*handler, 0, sizeof(struct tracex_stats_handler));
	free(*handler);
	*handler = NULL;
	}
}

tracex_stats_ret_t tracex_stats_load_script(tracex_stats_handler_t *handler, const char *script_path)
{
	tracex_stats_ret_t status;

	if (script_path == NULL) {
		status = TRACEX_STATS_BAD_INPUT_PTR;
		goto handle_exit;
	}

    if ((status = check_handle_valid(handler)) != TRACEX_STATS_SUCCESS)
	    goto handle_exit;

	if ((status = tracex_stats_scripting_load_script(handler, script_path)) != TRACEX_STATS_SUCCESS)
		goto handle_exit;

handle_exit:
	return status;
}

tracex_stats_ret_t tracex_stats_get_lua_err_str(tracex_stats_handler_t *handler, const char **lua_err_str)
{
	tracex_stats_ret_t status;
	const char *err_str;

	err_str = NULL;

    if ((status = check_handle_valid(handler)) != TRACEX_STATS_SUCCESS)
	    goto handle_exit;

    err_str = handler->lua_err_str;

    status = TRACEX_STATS_SUCCESS;

handle_exit:
	*lua_err_str = err_str;
	return status;
}

tracex_stats_ret_t tracex_stats_push_object(tracex_stats_handler_t *handler, const struct tracex_stats_object *object)
{
	tracex_stats_ret_t status;

	if (object == NULL) {
		status = TRACEX_STATS_BAD_INPUT_PTR;
		goto handle_exit;
	}

    if ((status = check_handle_valid(handler)) != TRACEX_STATS_SUCCESS)
	    goto handle_exit;

	status = tracex_stats_scripting_push_object(handler, object);

handle_exit:
	return status;
}
tracex_stats_ret_t tracex_stats_push_event(tracex_stats_handler_t *handler, const struct tracex_stats_event *event)
{
	tracex_stats_ret_t status;

	if (event == NULL) {
		status = TRACEX_STATS_BAD_INPUT_PTR;
		goto handle_exit;
	}

    if ((status = check_handle_valid(handler)) != TRACEX_STATS_SUCCESS)
	    goto handle_exit;

    status = tracex_stats_scripting_push_event(handler, event);

handle_exit:
	return status;

}

tracex_stats_ret_t tracex_stats_get_results(tracex_stats_handler_t *handler, tracex_stats_results **results)
{
    tracex_stats_ret_t status;

    if (results == NULL) {
        status = TRACEX_STATS_BAD_INPUT_PTR;
        goto handle_exit;
    }
    *results = NULL;

    if ((status = check_handle_valid(handler)) != TRACEX_STATS_SUCCESS)
        goto handle_exit;

    status = tracex_stats_scripting_get_results(handler, results);

handle_exit:
    return status;
}

void tracex_stats_results_free(tracex_stats_results **results)
{
    size_t r;
    size_t c;
    tracex_stats_results *res;

    if (results == NULL || *results == NULL)
        return;

    res = *results;

    if (res->rows != NULL) {
        for (r = 0; r < res->n_rows; r++) {
            if (res->rows[r].values != NULL) {
                for (c = 0; c < res->n_columns; c++) {
                    if (res->columns != NULL &&
                        res->columns[c].type == COL_STRING)
                        free(res->rows[r].values[c].str);
                }
                free(res->rows[r].values);
            }
        }
        free(res->rows);
    }

    free(res->columns);
    free(res);
    *results = NULL;
}

static tracex_stats_ret_t check_handle_valid(tracex_stats_handler_t *handler)
{
    if (handler != NULL && handler->lua_state != NULL)
	    return TRACEX_STATS_SUCCESS;
    else
	    return TRACEX_STATS_INVALID_LUA_HANDLER;
}