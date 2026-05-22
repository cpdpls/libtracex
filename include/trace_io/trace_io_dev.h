#ifndef __TRACE_IO_DEV_H__
#define __TRACE_IO_DEV_H__

#include <stdlib.h>
#include "trace_errno.h"

typedef TRACERet_t (*readFunc)(size_t len, size_t *bytes_read, void *buf);

struct trace_io_dev
{
    void *io_context;
    readFunc read;
};

#endif