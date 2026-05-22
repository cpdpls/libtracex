#include "trace_io.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

struct trace_io_tcp_context
{
    struct sockaddr_in address;
    uint16_t port;
    int timeout_sec;
    const char *host;
};
struct trace_io_dev *trace_io_from_tcp(const char *host, uint16_t port, int timeout_sec)
{
    return NULL;
}