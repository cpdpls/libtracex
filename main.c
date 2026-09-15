#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "tracex.h"
#include "tracex_debug.h"


TRACEX_handler_t *handler;

void eventParsedCB(TRACEX_event_t *event, TRACEX_Ret_t status);
void headerParsedCB(struct TRACEX_header_t *header, TRACEX_Ret_t status);
void objectParsedCB(TRACEX_object_t *object, TRACEX_Ret_t status);

int main(void)
{
    struct sockaddr_in server;
    int lfd;
    int bytes_read;
    TRACEX_Ret_t status;
    char buffer[500];
    TRACEX_Callbacks_t callbacks;

    callbacks.EventParsed = eventParsedCB;
    callbacks.HeaderParsed = headerParsedCB;
    callbacks.ObjectParsed = objectParsedCB;
    
    TRACEX_INIT();

    lfd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = htons(5555);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(lfd, (struct sockaddr *)&server, sizeof server) == -1)
    {
        printf("Failed to connect to the server !\n");
        return -1;
    }


    status = TRACEX_createHandler(&handler);

    if (status != TRACEX_SUCCESS)
    {
        printf("%s\n", TRACEX_strerror(status));
        return -1;
    }

    if (TRACEX_registerCallbacks(handler, &callbacks) != TRACEX_SUCCESS)
    {
        printf("%s\n", TRACEX_strerror(status));
        return -1;
    }

    do
    {
        bytes_read = recv(lfd, buffer, sizeof(buffer), 0);

        status = TRACEX_parse(handler, buffer, bytes_read);

    }while (status == TRACEX_NEED_MORE && bytes_read >= 0);

    

    TRACEX_destroyHandler(&handler);

}


void eventParsedCB(TRACEX_event_t *event, TRACEX_Ret_t status)
{

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