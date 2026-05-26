#include <stdio.h>
#include "trace_io.h"
#include "trace.h"

int main()
{
    struct trace_io_dev *dev;
    TRACERet_t status;
    status = trace_io_from_file(&dev, "trace.trx");

    if (status != TRACE_SUCCESS)
    {
        printf("%s\n", tracestrerror(status));
    }

    trace_io_destroy_dev(&dev);

}