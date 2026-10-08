#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdint.h>
#include <stdbool.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "tracex/tracex.h"
#include "tracex/tracex_debug.h"
#include "labels_engine/labels_engine.h"

typedef enum {
    COL_UINT32,
    COL_UINT64,
    COL_DOUBLE,
    COL_STRING,
    COL_BOOL
} ColumnType;

typedef struct {
    char        name[64];
    ColumnType  type;
    char        unit[32];
    char        description[128];
} ColumnMeta;

typedef struct {
    union {
        uint32_t u32;
        uint64_t u64;
        double   f64;
        char    *str;
        bool     b;
    } *values;
} ResultRow;

typedef struct {
    char          name[128];
    char          description[256];
    char          suggested_view[32];

    ColumnMeta   *columns;
    size_t        n_columns;

    ResultRow    *rows;
    size_t        n_rows;
} StatisticResult;

/* Global handler variable */
lua_State *L;
tracex_handler_t *handler;

void eventParsedCB(tracex_handler_t *handler, struct tracex_event *event, tracex_ret_t status);
void headerParsedCB(tracex_handler_t *handler, struct tracex_header *header, tracex_ret_t status);
void objectParsedCB(tracex_handler_t *handler, struct tracex_object *object, tracex_ret_t status);
static tracex_ret_t registerCallbacks(void);
static tracex_ret_t registerResolver(void);
static int initLabelsEngine(void);
static void uninitLabelsEngine(void);
static StatisticResult *lua_table_to_statistic_result(lua_State *L);
static void statistic_result_free(StatisticResult *res);
static void print_statistic_result(const StatisticResult *res);
static void push_object_to_lua(lua_State *L, const struct tracex_object *obj);
static void call_on_event(lua_State *L, const struct tracex_event *ev);
static void call_finalize(lua_State *L);

void file(char *file_path, char enable_random);

void eventParsedCB(tracex_handler_t *handler, struct tracex_event *event, tracex_ret_t status)
{
    if (status == TRACEX_SUCCESS)
    {
        call_on_event(L, event);
	}
	    /*TRACEX_debug_print_single_event(event); */

}
void headerParsedCB(tracex_handler_t *handler, struct tracex_header *header, tracex_ret_t status)
{
    if (status == TRACEX_SUCCESS) {
        TRACEX_debug_print_user_header(header);

        /* Give Lua the mask so it can compute c    orrect deltas */
        lua_pushinteger(L, header->timeStampMask);   // or whatever the field is called
        lua_setglobal(L, "timestamp_mask");
    }

}
void objectParsedCB(tracex_handler_t *handler, struct tracex_object *object, tracex_ret_t status)
{
    if (status == TRACEX_SUCCESS)
    {
	    push_object_to_lua(L, object);
	    // TRACEX_debug_print_single_object(object);
    }

}

static tracex_ret_t registerCallbacks(void)
{
    struct tracex_callbacks callbacks;
    tracex_ret_t status;

    callbacks.on_event_parsed = eventParsedCB;
    callbacks.on_header_parsed = headerParsedCB;
    callbacks.on_object_parsed = objectParsedCB;

    status = tracex_register_callbacks(handler, &callbacks);

    return status;
}

static tracex_ret_t registerResolver(void)
{
	return tracex_register_resolver_function(handler, labels_engine_resolve_labels);
}

static int initLabelsEngine(void)
{
	int status;

	/* We first load all the labels in the labels engine */

	status = labels_engine_object_load_labels(NULL);

	status |= labels_engine_event_load_labels(NULL);

	return status;
}

static void uninitLabelsEngine(void)
{
	labels_engine_event_destroy_labels();
	labels_engine_object_destroy_labels();
}

static StatisticResult *lua_table_to_statistic_result(lua_State *L)
{
	size_t i;
	size_t r;
	size_t c;
	if (!lua_istable(L, -1))
		return NULL;

	StatisticResult *res = calloc(1, sizeof(*res));
	if (!res)
		return NULL;

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
		res->n_columns = lua_rawlen(L, -1);
		res->columns = calloc(res->n_columns, sizeof(ColumnMeta));

		for (i = 0; i < res->n_columns; i++) {
			lua_rawgeti(L, -1, (int)(i + 1));

			lua_getfield(L, -1, "name");
			if (lua_isstring(L, -1))
				strncpy(res->columns[i].name, lua_tostring(L, -1), 63);
			lua_pop(L, 1);

			lua_getfield(L, -1, "type");
			const char *t = lua_tostring(L, -1);
			if (t) {
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
				strncpy(res->columns[i].unit, lua_tostring(L, -1), 31);
			lua_pop(L, 1);

			lua_pop(L, 1);
		}
    }
    lua_pop(L, 1);

    /* rows */
    lua_getfield(L, -1, "rows");
    if (lua_istable(L, -1)) {
        res->n_rows = lua_rawlen(L, -1);
        res->rows = calloc(res->n_rows, sizeof(ResultRow));

        for (r = 0; r < res->n_rows; r++) {
            lua_rawgeti(L, -1, (int)(r+1));
            res->rows[r].values = calloc(res->n_columns, sizeof(*res->rows[r].values));

            for (c = 0; c < res->n_columns; c++) {
                lua_rawgeti(L, -1, (int)(c+1));

                switch (res->columns[c].type) {
                    case COL_UINT32: res->rows[r].values[c].u32 = (uint32_t)lua_tointeger(L,-1); break;
                    case COL_UINT64: res->rows[r].values[c].u64 = (uint64_t)lua_tointeger(L,-1); break;
                    case COL_DOUBLE: res->rows[r].values[c].f64 = lua_tonumber(L,-1); break;
                    case COL_STRING: res->rows[r].values[c].str = strdup(lua_tostring(L,-1) ? lua_tostring(L,-1) : ""); break;
                    case COL_BOOL:   res->rows[r].values[c].b   = lua_toboolean(L,-1); break;
                }
                lua_pop(L, 1);
            }
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);

    return res;


}
static void statistic_result_free(StatisticResult *res)
{
	size_t r;
	size_t c;

	if (!res)
		return;
	for (r = 0; r < res->n_rows; r++) {
		if (res->rows[r].values) {
			for (c = 0; c < res->n_columns; c++)
				if (res->columns[c].type == COL_STRING)
					free(res->rows[r].values[c].str);
			free(res->rows[r].values);
		}
    }
    free(res->rows);
    free(res->columns);
    free(res);

}
static void print_statistic_result(const StatisticResult *res)
{
	size_t c;
	size_t r;

	if (!res)
		return;

	printf("===== Generic StatisticResult (GUI view) =====\n");
	printf("Name        : %s\n", res->name);
	printf("Description : %s\n", res->description);
	printf("View        : %s\n\n", res->suggested_view);

	for (c = 0; c < res->n_columns; c++)
		printf("%-20s", res->columns[c].name);
	printf("\n");
	for (c = 0; c < res->n_columns * 14; c++)
		putchar('-');
	printf("\n");

	for (r = 0; r < res->n_rows; r++) {
		for (c = 0; c < res->n_columns; c++) {
			switch (res->columns[c].type) {
			case COL_UINT32:
				printf("%-20u", res->rows[r].values[c].u32);
				break;
			case COL_UINT64:
				printf("%-20llu", (unsigned long long)res->rows[r].values[c].u64);
				break;
			case COL_DOUBLE:
				printf("%-20.2f", res->rows[r].values[c].f64);
				break;
			case COL_STRING:
				printf("%-20s", res->rows[r].values[c].str ? res->rows[r].values[c].str : "");
				break;
			case COL_BOOL:
				printf("%-20s", res->rows[r].values[c].b ? "true" : "false");
				break;
			}
		}
		printf("\n");
	}
    printf("\n");

}
static void push_object_to_lua(lua_State *L, const struct tracex_object *obj)
{
    lua_getglobal(L, "objects");
    if (!lua_istable(L, -1)) {
        /* table does not exist yet → create it */
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);          /* duplicate */
        lua_setglobal(L, "objects");   /* objects = {} */
    }

    /* objects[ptr] = { ptr = ..., type = ..., name = "..." } */
    lua_pushinteger(L, obj->pointer);      /* key */

    lua_newtable(L);                   /* value table */
    lua_pushinteger(L, obj->pointer);
    lua_setfield(L, -2, "ptr");

    lua_pushinteger(L, obj->type);
    lua_setfield(L, -2, "type");

    lua_pushstring(L, obj->name);
    lua_setfield(L, -2, "name");

    lua_settable(L, -3);               /* objects[ptr] = value */
    lua_pop(L, 1);                     /* pop the objects table */

}
static void call_on_event(lua_State *L, const struct tracex_event *ev)
{

    lua_getglobal(L, "on_event");
    if (!lua_isfunction(L, -1)) {
        fprintf(stderr, "on_event is not a function\n");
        lua_pop(L, 1);
        return;
    }

    /* Order of arguments passed to Lua:
     * 1. eventId
     * 2. threadPointer          ← the running thread
     * 3. threadPriority
     * 4. info1
     * 5. info2
     * 6. info3
     * 7. info4
     * 8. timeStamp
     */
    lua_pushinteger(L, ev->eventId);
    lua_pushinteger(L, ev->threadPointer);
    lua_pushinteger(L, ev->threadPriority);
    lua_pushinteger(L, ev->rawInfos.info1);
    lua_pushinteger(L, ev->rawInfos.info2);
    lua_pushinteger(L, ev->rawInfos.info3);
    lua_pushinteger(L, ev->rawInfos.info4);
    lua_pushinteger(L, ev->timeStamp);

    if (lua_pcall(L, 8, 0, 0) != LUA_OK) {
        fprintf(stderr, "error in on_event: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
    }

}
static void call_finalize(lua_State *L)
{
    lua_getglobal(L, "finalize");
    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            fprintf(stderr, "error in finalize: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    } else {
        lua_pop(L, 1);
    }

}
void file(char *file_path, char enable_random)
{
    FILE *file_ptr;
    char *buffer;
    char *orig_buff;
    size_t trace_size;
    tracex_ret_t status;
    size_t bytes_read;
    size_t left;
    size_t bytes_left;

    size_t bytes_to_parse;

    if (enable_random)
    {
        srand(time(NULL));
    }
    file_ptr = fopen(file_path, "rb");

    if (file_ptr == NULL)
    {
        printf("Failed to open the trace file !\n");
        exit(-1);
    }

    fseek(file_ptr, 0L, SEEK_END);
    trace_size = ftell(file_ptr);
    fseek(file_ptr, 0L, SEEK_SET);

    buffer = (char*)malloc(sizeof(char) * trace_size);
    if (buffer == NULL)
    {
        printf("Allocation failure in test !\n");
        exit(-1);
    }
    orig_buff = buffer;

    fread(buffer, trace_size, 1, file_ptr);

    left = trace_size;

    while (left != 0) {

        if (enable_random){
            bytes_to_parse = rand() % 100 + 1;
            if (bytes_to_parse > left)
                bytes_to_parse = left;
        }
        else
        {
            bytes_to_parse = trace_size;
        }
        bytes_left = bytes_to_parse;
        do {

            status = tracex_parse(handler, buffer, bytes_left, &bytes_read);
            buffer += bytes_read;
            bytes_left -= bytes_read;
            left -= bytes_read;

        } while (bytes_left != 0);

    }

    TRACEX_debug_print_total_events_count(handler);

    call_finalize(L);

     /* 5. Get result and convert to generic structure */
    lua_getglobal(L, "get_result");
    if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
        fprintf(stderr, "error in get_result: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        lua_close(L);
        return;
    }


    StatisticResult *res = lua_table_to_statistic_result(L);
    lua_pop(L, 1);

    /* 6. This is what the GUI receives.
     *    The first column (object_ptr) lets the GUI find the real object.
     */
    print_statistic_result(res);

    statistic_result_free(res);
    lua_close(L);


    free(orig_buff);
    fclose(file_ptr);
    
}

int main(int argc, char *argv[])
{
    tracex_ret_t status;
    int appstatus;
    char *file_path = NULL;
    char enable_random = 0;
    int val, index = 0;
    const struct option lopts[] = {
        {"file",    required_argument,  NULL,  'f' },
        {"random",  no_argument,        NULL,  'r' },
        {NULL,      no_argument,        NULL,   0 }
    };

    while (EOF != (val = getopt_long(argc, argv, ":f:r", lopts, &index))) {
	    switch (val) {
            case 'f':
                file_path = optarg;
                break;
            case 'r':
                enable_random = 1;
                break;
    
            case '?':
                printf("Unknown option '%c'\n", optopt);
                break;
            case ':':
                printf("Missing argument for option : '%c'\n", optopt);
                exit(1);
                break;

	    default:
		        printf("wtf\n");
            break;
	    }
    }

    if (file_path == NULL) {
	    printf("Missing required file path !\n");
	    exit(1);
    }
    if (optind < argc) {
        printf("non-option ARGV-elements: ");
               while (optind < argc)
                   printf("%s ", argv[optind++]);
               printf("\n");
	       exit(1);
    }

    L = luaL_newstate();
    if (!L) {
        fprintf(stderr, "cannot create Lua state\n");
        return 1;
    }
    luaL_openlibs(L);


    /* 3. Load the statistic script */
    if (luaL_dofile(L, "examples/parse-from-file/test.lua") != LUA_OK) {
        fprintf(stderr, "error loading script: %s\n", lua_tostring(L, -1));
        lua_close(L);
        return 1;
    }


   /* First we create a handler for the parsing */
    status = tracex_create_new_handler(&handler);

    if (status != TRACEX_SUCCESS)
    {
        printf("%s\n", TRACEX_strerror(status));
	    appstatus = 1;
	    goto handle_exit;
    }

    /* We now need to register the callbacks, otherwise we won't be notified about newly parsed object or event */
    if ((status = registerCallbacks()) != TRACEX_SUCCESS) {
        printf("%s\n", TRACEX_strerror(status));
	    appstatus = 1;
	    goto handle_exit;
    }

    if (initLabelsEngine() != 0){
	    printf("An error occured at labels initialization !\n");
	    appstatus = 1;
	    goto handle_exit;
    }

    if ((status = registerResolver()) != TRACEX_SUCCESS) {
        printf("%s\n", TRACEX_strerror(status));
	    appstatus = 1;
	    goto handle_exit;
    }
    
    file(file_path, enable_random);

    uninitLabelsEngine();

handle_exit:
    tracex_destroy_handler(&handler);
    return appstatus;

}