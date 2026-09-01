#ifndef __TX_TRACE_HEADER_H__
#define __TX_TRACE_HEADER_H__

#include <stdint.h>
#include "tx_trace_errno.h"
#include "tx_def.h"

struct tracex_header_t
{
    uint32_t header_id;
    

};

TXTRACE_Ret_t traceX_parse_header(struct tracex_handler_t handler, struct tracex_header_t **header);
#endif