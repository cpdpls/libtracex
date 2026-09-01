#include "tx_trace.h"
#include "abstractio/aio.h"

void TX_TRACE_INIT(void)
{
    AIO_BOOTSTRAP();
}

void TX_TRACE_EXIT(void)
{
    AIO_EXIT();
}