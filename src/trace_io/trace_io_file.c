#include "trace_io.h"

#include <stdio.h>

struct trace_io_file_context
{
    FILE *descriptor;
    char *path;
    char *mode;
};

struct trace_io_dev *trace_io_from_file(const char *path, const char *mode)
{
    return NULL;

}