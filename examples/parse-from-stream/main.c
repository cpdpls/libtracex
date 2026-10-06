#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

void network();
void file(char random);

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
        TRACEX_debug_print_single_object(object);
    }

}

void network()
{
    struct sockaddr_in server;
    int lfd;
    size_t to_parse;
    size_t bytes_parsed;
    size_t bytes_left;
    tracex_ret_t status;
    char buffer[500];
    char *buff_ptr;
    

    lfd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = htons(5555);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(lfd, (struct sockaddr *)&server, sizeof server) == -1)
    {
        printf("Failed to connect to the server !\n");
        return;
    }

    do
    {
	    // tracex_refresh_resolver_labels(handler);
	    to_parse = recv(lfd, buffer, sizeof(buffer), 0);
	    buff_ptr = buffer;

	    bytes_left = to_parse;
	    do {
		    status = tracex_parse(handler, buff_ptr, bytes_left, &bytes_parsed);
		    buff_ptr += bytes_parsed;
		    bytes_left -= bytes_parsed;

	    } while (bytes_left != 0);

    }while (to_parse > 0);

    close(lfd);

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

int main(void)
{
    tracex_ret_t status;
    int appstatus;

    char key;
    
    /* First we create a handler for the parsing */
    status = tracex_create_new_handler(&handler);

    if (status != TRACEX_SUCCESS)
    {
        printf("%s\n", TRACEX_strerror(status));
	    appstatus = 1;
	    goto handle_error;
    }

    /* We now need to register the callbacks, otherwise we won't be notified about newly parsed object or event */
    if ((status = registerCallbacks()) != TRACEX_SUCCESS) {
        printf("%s\n", TRACEX_strerror(status));
	    appstatus = 1;
	    goto handle_error;
    }

    if (initLabelsEngine() != 0){
	    printf("An error occured at labels initialization !\n");
	    appstatus = 1;
	    goto handle_error;
    }

    if ((status = registerResolver()) != TRACEX_SUCCESS) {
        printf("%s\n", TRACEX_strerror(status));
	    appstatus = 1;
	    goto handle_error;
    }

    /* Call the main network logic to parse incrementally */
    network();

    /* Destroy the loaded labels */
    uninitLabelsEngine();

handle_error:
	tracex_destroy_handler(&handler);

handle_exit:
	return 0;

}
