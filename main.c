#include <stdio.h>
#include "trace_io_fs.h"

struct trace_io_fs_ops ops;

int main()
{
    ops.close = NULL;
    ops.open = NULL;
    ops.read = NULL;

}