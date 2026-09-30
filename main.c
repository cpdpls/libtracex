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

#include "tracex.h"
#include "tracex_debug.h"


tracex_handler *handler;

void eventParsedCB(tracex_handler_t *handler, struct tracex_event *event, tracex_ret_t status);
void headerParsedCB(tracex_handler_t *handler, struct tracex_header *header, tracex_ret_t status);
void objectParsedCB(tracex_handler_t *handler, struct tracex_object *object, tracex_ret_t status);
void network();
void file(char random);

int main(void)
{
    tracex_ret_t status;
    struct tracex_callbacks callbacks;

    callbacks.on_event_parsed = eventParsedCB;
    callbacks.on_header_parsed = headerParsedCB;
    callbacks.on_object_parsed = objectParsedCB;

    tracex_init();

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
    
    file(0);
    tracex_destroy_handler(&handler);

    tracex_deinit();

    return 0;

}

void eventParsedCB(tracex_handler_t *handler, struct tracex_event *event, tracex_ret_t status)
{
    if (status == TRACEX_SUCCESS)
    {
        // TRACEX_debug_print_single_event(event);
    }

}
void headerParsedCB(tracex_handler_t *handler, struct tracex_header *header, tracex_ret_t status)
{
    if (status == TRACEX_SUCCESS)
    {
        TRACEX_debug_print_raw_header(header);
    }

}
void objectParsedCB(tracex_handler_t *handler, struct tracex_object *object, tracex_ret_t status)
{
    if (status == TRACEX_SUCCESS)
    {
        // TRACEX_debug_print_single_object(object);
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

void file(char random)
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

    if (random)
    {
        srand(time(NULL));
    }
    file_ptr = fopen("trace_big.trx", "rb");

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

        if (random){
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

            status = tracex_parse(handler, buffer, bytes_to_parse, &bytes_read);
            buffer += bytes_read;
            bytes_left -= bytes_read;
            left -= bytes_read;

        } while (bytes_left != 0);

    }

    free(orig_buff);
    fclose(file_ptr);
    
}