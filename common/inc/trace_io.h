#ifndef __TRACE_IO_H__
#define __TRACE_IO_H__

#include <stdlib.h>
#include "trace_errno.h"

/* Forward declaration of an internal type */

typedef struct trace_io_dev trace_io_dev;

struct trace_io_dev *trace_io_from_file(const char *path, const char *mode);
struct trace_io_dev *trace_io_from_socket(const char *host, uint16_t port, int timeout_sec);


#endif