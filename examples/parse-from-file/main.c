#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdint.h>
#include <stdbool.h>

#include "tracex/tracex.h"
#include "tracex/tracex_debug.h"
#include "labels_engine/labels_engine.h"
#include "tracex_stats/tracex_stats.h"
#include "tracex_stats/tracex_stats_errno.h"

/* Global handler variable */
tracex_handler_t *handler;
tracex_stats_handler_t *stats_handler;

void eventParsedCB(tracex_handler_t *handler, struct tracex_event *event, tracex_ret_t status);
void headerParsedCB(tracex_handler_t *handler, struct tracex_header *header, tracex_ret_t status);
void objectParsedCB(tracex_handler_t *handler, struct tracex_object *object, tracex_ret_t status);
static tracex_ret_t registerCallbacks(void);
static tracex_ret_t registerResolver(void);
static int initLabelsEngine(void);
static void uninitLabelsEngine(void);
static void print_statistic_result(const tracex_stats_results *res);

void file(char *file_path, char enable_random);

void eventParsedCB(tracex_handler_t *handler, struct tracex_event *event, tracex_ret_t status)
{
	struct tracex_stats_event event_stats;

	if (status == TRACEX_SUCCESS) {
		event_stats.event_id = event->eventId;
		event_stats.thread_pointer = event->threadPointer;
		event_stats.thread_priority = event->threadPriority;
		event_stats.i1 = event->rawInfos.info1;
        event_stats.i2 = event->rawInfos.info2;
        event_stats.i3 = event->rawInfos.info3;
        event_stats.i4 = event->rawInfos.info4;
	    event_stats.timestamp = event->timeStamp;

	    tracex_stats_push_event(stats_handler, &event_stats);
	    // call_on_event(L, event);
	}
	    /*TRACEX_debug_print_single_event(event); */

}
void headerParsedCB(tracex_handler_t *handler, struct tracex_header *header, tracex_ret_t status)
{
	if (status == TRACEX_SUCCESS) {
		TRACEX_debug_print_user_header(header);

		// /* Give Lua the mask so it can compute c    orrect deltas */
		// lua_pushinteger(L, header->timeStampMask);   // or whatever the field is called
		// lua_setglobal(L, "timestamp_mask");
	}
}
void objectParsedCB(tracex_handler_t *handler, struct tracex_object *object, tracex_ret_t status)
{
    struct tracex_stats_object obj_stats;

    if (status == TRACEX_SUCCESS)
    {
	    obj_stats.name = object->name;
	    obj_stats.pointer = object->pointer;
	    obj_stats.type = object->type;

	    TRACEX_debug_print_single_object(object);
	    tracex_stats_push_object(stats_handler, &obj_stats);
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

static void print_statistic_result(const tracex_stats_results *res)
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
    tracex_stats_results *res;

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

    tracex_stats_get_results(stats_handler, &res);

    print_statistic_result(res);

    tracex_stats_results_free(&res);

    tracex_stats_destroy_handler(&stats_handler);
    free(orig_buff);
    fclose(file_ptr);
    
}

int main(int argc, char *argv[])
{
    tracex_ret_t status;
    tracex_stats_ret_t stats_sts;
    const char *lua_str_error = NULL;
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

    if ((stats_sts = tracex_stats_create_new_handler(&stats_handler)) != TRACEX_STATS_SUCCESS) {
	    printf("%s\n", tracex_stats_strerror(stats_sts));
	    appstatus = 1;
	    goto handle_exit;
    }

    stats_sts = tracex_stats_load_script(stats_handler, "scripts/isr_enter_count.lua");
    if (stats_sts != TRACEX_STATS_SUCCESS) {
        printf("%s\n", tracex_stats_strerror(stats_sts));
        if (stats_sts == TRACEX_STATS_LUA_ERROR) {
		    tracex_stats_get_lua_err_str(stats_handler, &lua_str_error);
		    printf("LUA : %s\n", lua_str_error);
	}

	appstatus = 1;
	goto handle_exit;
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

    appstatus = 0;

handle_exit:
    tracex_destroy_handler(&handler);
    return appstatus;

}