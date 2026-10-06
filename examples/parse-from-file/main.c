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


/* Global handler variable */
tracex_handler_t *handler;

void eventParsedCB(tracex_handler_t *handler, struct tracex_event *event, tracex_ret_t status);
void headerParsedCB(tracex_handler_t *handler, struct tracex_header *header, tracex_ret_t status);
void objectParsedCB(tracex_handler_t *handler, struct tracex_object *object, tracex_ret_t status);
void file(char *file_path, char enable_random);

static struct option long_options[] = {
    {"file",    required_argument, 0,  'f' },
    {"random",  no_argument,       0,  'r' },
    {0,         0,                 0,  0 }
};

int main(int argc, char *argv[])
{
    tracex_ret_t status;
    struct tracex_callbacks callbacks;
    int opt;
    int option_index = 0;
    char missing = 0;
    char *file_path;
    char enable_random = 0;

    while(1) {

        opt = getopt_long(argc, argv, "f:r", long_options, &option_index);

        if (opt == -1)
            break;
        
        switch(opt) {
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
		    printf("Missing argument !\n");
                break;
            
            default:
		    printf("Unexpected getopt_long\n");
                break;
	    }
    }

    callbacks.on_event_parsed = eventParsedCB;
    callbacks.on_header_parsed = headerParsedCB;
    callbacks.on_object_parsed = objectParsedCB;

    status = tracex_create_new_handler(&handler);

    if (status != TRACEX_SUCCESS)
    {
        printf("%s\n", TRACEX_strerror(status));
        exit(-1);
    }

    if (tracex_register_callbacks(handler, &callbacks) != TRACEX_SUCCESS)
    {
        printf("%s\n", TRACEX_strerror(status));
        exit(-1);
    }
    
    file(file_path, enable_random);

    tracex_destroy_handler(&handler);

    return 0;

}

void eventParsedCB(tracex_handler_t *handler, struct tracex_event *event, tracex_ret_t status)
{
    if (status == TRACEX_SUCCESS)
    {
        //TRACEX_debug_print_single_event(event);
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
        // TRACEX_debug_print_single_object(object);
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

    free(orig_buff);
    fclose(file_ptr);
    
}