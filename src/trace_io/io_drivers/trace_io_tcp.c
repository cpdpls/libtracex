#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "trace_io.h"
#include "trace_io_fs.h"

struct trace_io_tcp_context
{
    struct sockaddr_in address;
    uint16_t port;
    int timeout_sec;
    const char *host;
};

static TRACERet_t test(void);

TRACERet_t test(void)
{

}
TRACE_IO_FS_DRIVER_INIT(test);
