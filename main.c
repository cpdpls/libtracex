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

#include "tracex.h"
#include "tracex_debug.h"


TRACEX_handler_t *handler;

void eventParsedCB(TRACEX_event_t *event, TRACEX_Ret_t status);
void headerParsedCB(struct TRACEX_header_t *header, TRACEX_Ret_t status);
void objectParsedCB(TRACEX_object_t *object, TRACEX_Ret_t status);
void network();
void file(char random);

int main(void)
{
    TRACEX_Ret_t status;
    TRACEX_Callbacks_t callbacks;

    callbacks.EventParsed = eventParsedCB;
    callbacks.HeaderParsed = headerParsedCB;
    callbacks.ObjectParsed = objectParsedCB;

    TRACEX_INIT();

    status = TRACEX_createHandler(&handler);

    if (status != TRACEX_SUCCESS)
    {
        printf("%s\n", TRACEX_strerror(status));
        exit(-1);
    }

    if (TRACEX_registerCallbacks(handler, &callbacks) != TRACEX_SUCCESS)
    {
        printf("%s\n", TRACEX_strerror(status));
        exit(-1);
    }
    
    network();
    TRACEX_destroyHandler(&handler);

    

}


void eventParsedCB(TRACEX_event_t *event, TRACEX_Ret_t status)
{
    if (status == TRACEX_SUCCESS)
    {
        TRACEX_debug_print_single_event(event);
    }

}
void headerParsedCB(struct TRACEX_header_t *header, TRACEX_Ret_t status)
{
    if (status == TRACEX_SUCCESS)
    {
        TRACEX_debug_print_raw_header(handler);
    }

}
void objectParsedCB(TRACEX_object_t *object, TRACEX_Ret_t status)
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
    int bytes_read;
    TRACEX_Ret_t status;
    char buffer[500];
    

    lfd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = htons(5555);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(lfd, (struct sockaddr *)&server, sizeof server) == -1)
    {
        printf("Failed to connect to the server !\n");
        exit(-1);
    }

    do
    {
        bytes_read = recv(lfd, buffer, sizeof(buffer), 0);

        status = TRACEX_parse(handler, buffer, bytes_read);

    }while (status == TRACEX_NEED_MORE && bytes_read >= 0);

}

void file(char random)
{
    FILE *file_ptr;
    char *buffer;
    char *orig_buff;
    size_t trace_size;
    TRACEX_Ret_t status;
    size_t bytes_to_parse;

    if (random)
    {
        srand(time(NULL));
    }
    file_ptr = fopen("r15b_tracex_dump.trx", "rb");

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

    do
    {
        if (random)
        {
            bytes_to_parse = (rand() % 100) + 1;

        }
        else
        {
            bytes_to_parse = trace_size;
        }

        printf("Parsing %d\n", bytes_to_parse);
        status = TRACEX_parse(handler, buffer, bytes_to_parse);
        buffer += bytes_to_parse;

    } while (status == TRACEX_NEED_MORE);


    free(orig_buff);
    fclose(file_ptr);
    
}