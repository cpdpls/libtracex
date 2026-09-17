#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <time.h>

#include "libsockets/include/sockets.h"

void send_func(int client_socket, char *buffer, size_t buff_size);

int main()
{
    int serv_socket;
    int client_socket;
    char *buffer;
    size_t trace_size;
    FILE *trace;
    
    
    srand(time(NULL));
    
    trace = fopen("trace.trx", "rb");
    
    if (trace == NULL)
    {
        printf("Failed to open the trace file !\n");
        return -1;
    }

    fseek(trace, 0L, SEEK_END);
    trace_size = ftell(trace);
    fseek(trace, 0L, SEEK_SET);

    buffer = malloc(trace_size);
    fread(buffer, trace_size, 1, trace);

    serv_socket = Create_server(4444);
    if (serv_socket == -1)
    {
        printf("Failed to create the server !\n");
        free(buffer);
        fclose(trace);
    }

    while (1)
    {
        client_socket = Accept_connexion(serv_socket);
        send_func(client_socket, buffer, trace_size);

    }

}

void send_func(int client_socket, char *buffer, size_t buff_size)
{
    int status;
    int data_size;
    size_t bytes_left;

    bytes_left = buff_size;

    do
    {
        data_size = rand() % 500 + 1;
        if (bytes_left < data_size)
        {
            data_size = bytes_left;
        }

        status = Send_msg(client_socket, buffer, data_size);
        sleep(0.1);
        printf("Sended %d bytes | left : %d\n", data_size, bytes_left);
        bytes_left -= data_size;

    }while (status >= 0 && bytes_left != 0);

    close(client_socket);
    printf("Closing connection !\n");

}