
#ifndef TRACEX_STATS
#define TRACEX_STATS

#include <stdint.h>
#include <stddef.h>
#include "tracex_stats_errno.h"


typedef struct tracex_stats_handler tracex_stats_handler_t;


enum tracex_stats_column_type {
    COL_UINT32,
    COL_UINT64,
    COL_DOUBLE,
    COL_STRING,
    COL_BOOL
};

struct tracex_stats_column {
    char        name[64];
    enum tracex_stats_column_type  type;
    char        unit[32];
    char        description[128];
};

struct tracex_stats_row {
    union {
        uint32_t u32;
        uint64_t u64;
        double   f64;
        char    *str;
        int     b;
    } *values;
};

struct tracex_stats_object {
    uint32_t type;
    uint32_t pointer;
    uint8_t     *name;
    uint32_t param1;
	uint32_t param2;
};

struct tracex_stats_event {
    uint32_t event_id;
    uint32_t thread_pointer;
    uint32_t thread_priority;
    uint32_t i1;
    uint32_t i2;
    uint32_t i3;
    uint32_t i4;
    uint64_t timestamp;
};


typedef struct {
    char          name[128];
    char          description[256];
    char          suggested_view[32];

    struct tracex_stats_column   *columns;
    size_t        n_columns;

    struct tracex_stats_row    *rows;
    size_t        n_rows;
} tracex_stats_results;

tracex_stats_ret_t tracex_stats_create_new_handler(tracex_stats_handler_t **new_handler);
void tracex_stats_destroy_handler(tracex_stats_handler_t **handler);
tracex_stats_ret_t tracex_stats_load_script(tracex_stats_handler_t *handler, const char *script_path);
tracex_stats_ret_t tracex_stats_get_lua_err_str(tracex_stats_handler_t *handler, const char **lua_err_str);

tracex_stats_ret_t tracex_stats_push_object(tracex_stats_handler_t *handler, const struct tracex_stats_object *object);
tracex_stats_ret_t tracex_stats_push_event(tracex_stats_handler_t *handler, const struct tracex_stats_event *event);
tracex_stats_ret_t tracex_stats_get_results(tracex_stats_handler_t *handler, tracex_stats_results **results);
void tracex_stats_results_free(tracex_stats_results **results);
#endif