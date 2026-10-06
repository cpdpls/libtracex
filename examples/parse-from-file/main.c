#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
 #include <getopt.h>

#include "tracex/tracex.h"
#include "tracex/tracex_debug.h"
#include "labels_engine/labels_engine.h"

/* Global handler variable */
tracex_handler_t *handler;

void eventParsedCB(tracex_handler_t *handler, struct tracex_event *event, tracex_ret_t status);
void headerParsedCB(tracex_handler_t *handler, struct tracex_header *header, tracex_ret_t status);
void objectParsedCB(tracex_handler_t *handler, struct tracex_object *object, tracex_ret_t status);
static tracex_ret_t registerCallbacks(void);
static tracex_ret_t registerResolver(void);
static int initLabelsEngine(void);
static void uninitLabelsEngine(void);

void file(char *file_path, char enable_random);

void eventParsedCB(tracex_handler_t *handler, struct tracex_event *event, tracex_ret_t status)
{
    if (status == TRACEX_SUCCESS)
    {
        /*TRACEX_debug_print_single_event(event); */
    }

}
void headerParsedCB(tracex_handler_t *handler, struct tracex_header *header, tracex_ret_t status)
{
    if (status == TRACEX_SUCCESS)
    {
        TRACEX_debug_print_user_header(header);
    }

}
void objectParsedCB(tracex_handler_t *handler, struct tracex_object *object, tracex_ret_t status)
{
    if (status == TRACEX_SUCCESS)
    {
        TRACEX_debug_print_single_object(object);
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

handle_exit:
    tracex_destroy_handler(&handler);
    return appstatus;

}