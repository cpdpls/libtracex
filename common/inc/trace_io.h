#ifndef __TRACE_IO_H__
#define __TRACE_IO_H__

#include <stdlib.h>
#include <stdint.h>
#include "trace_errno.h"

/* Forward declaration of the internal dev structure */

typedef struct trace_io_fs trace_io_fs;

enum io_whence
{
    IO_WHENCE_SET,
    IO_WHENCE_CUR,
    IO_WHENCE_END

};

trace_io_fs *trace_io_open(const char *path);

TRACERet_t trace_io_close(struct trace_io_fs *io_fs);

TRACERet_t trace_io_read(struct trace_io_fs *io_fs, size_t len, size_t *bytes_read, void *buf);

TRACERet_t trace_io_peek(struct trace_io_fs *io_fs, size_t len, size_t *bytes_read, void *buf);

TRACERet_t trace_io_tell(struct trace_io_fs *io_fs, size_t *offset);

TRACERet_t trace_io_seek(struct trace_io_fs *io_fs, size_t offset , enum io_whence whence);

#endif