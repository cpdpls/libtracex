#ifndef __TRACE_IO_INTERNAL_H__
#define __TRACE_IO_INTERNAL_H__

#include "trace_io_fs.h"

struct trace_io_fs
{
    const struct trace_io_fs_ops *ops;
    void *private_data;
};

#endif