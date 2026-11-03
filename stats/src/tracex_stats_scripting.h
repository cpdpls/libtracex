#ifndef TRACEX_STATS_SCRIPTING_H
#define TRACEX_STATS_SCRIPTING_H

#include "tracex_stats/tracex_stats.h"

tracex_stats_ret_t tracex_stats_scripting_load_script(tracex_stats_handler_t *handler, const char *script_path);
tracex_stats_ret_t tracex_stats_scripting_push_object(tracex_stats_handler_t *handler, const struct tracex_stats_object *object);
tracex_stats_ret_t tracex_stats_scripting_push_event(tracex_stats_handler_t *handler, const struct tracex_stats_event *event);
tracex_stats_ret_t tracex_stats_scripting_get_results(tracex_stats_handler_t *handler, tracex_stats_results **results);
#endif