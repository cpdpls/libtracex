#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "trace_io.h"
#include "trace_io_dev.h"

struct trace_io_tcp_context
{
    struct sockaddr_in address;
    uint16_t port;
    int timeout_sec;
    const char *host;
};
TRACERet_t trace_io_from_tcp(struct trace_io_dev **io_dev, tcp_info *tcp_info)
{
    return TRACE_SUCCESS;
}