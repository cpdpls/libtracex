#ifndef __TRACE_IO_FS_H__
#define __TRACE_IO_FS_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "trace_errno.h"

typedef struct trace_io_fs trace_io_fs;

struct trace_io_fs_ops
{
    TRACERet_t (*open)(trace_io_fs *, const char *path);
    TRACERet_t (*close)(trace_io_fs * fs);
    TRACERet_t (*read)(trace_io_fs * fs, size_t len, size_t *bytes_read, void *buf);
};

TRACERet_t trace_io_fs_register(const char *scheme, const struct trace_io_fs_ops *ops);
void trace_io_fs_unregister(const char *scheme);
void trace_io_fs_unregister_all(void);
#endif