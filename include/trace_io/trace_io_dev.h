#ifndef __TRACE_IO_DEV_H__
#define __TRACE_IO_DEV_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "trace_errno.h"


struct trace_io_file
{
    FILE *descriptor;
    char *path;
    size_t file_size;
};

struct trace_io_tcp
{
    struct sockaddr_in address;
    uint16_t port;
    int timeout_sec;
    const char *host;
};

enum dev_type
{
    DEV_FILE,
    DEV_TCP,
    DEV_UNKNOWN,
};

struct trace_io_dev
{
    union
    {
        struct trace_io_file file_dev;
        struct trace_io_tcp tcp_dev;
    };
    enum dev_type dev_type;
    TRACERet_t (*read)(size_t len, size_t *bytes_read, void *buf);
    void (*destroy)(struct trace_io_dev **dev);
};

struct trace_io_dev *create_io_dev(void);
void destroy_io_dev(struct trace_io_dev **dev);
#endif