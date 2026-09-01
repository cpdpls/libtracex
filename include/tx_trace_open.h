#ifndef TX_TRACE_OPEN_H
#define TX_TRACE_OPEN_H

#include <stdint.h>
#include "tx_trace_errno.h"


enum tx_scheme_type_t
{
    E_TX_SCHEME_STREAM_TYPE,
    E_TX_SCHEME_PHYSICAL_TYPE,
    E_TX_SCHEME_INVALID_TYPE,
};
struct tx_scheme_descr_t
{
    uint8_t *scheme_name;
    enum tx_scheme_type_t type;  
};
struct tx_schemes_descriptor_list_t
{
    struct tx_scheme_descr_t **schemes;
    uint32_t count;
};


TXTRACE_Ret_t traceX_open(const char *fullpath);
TXTRACE_Ret_t traceX_getSchemesDescriptors(struct tx_schemes_descriptor_list_t **descriptors);
void traceX_destroySchemesDescriptors(struct tx_schemes_descriptor_list_t **descriptors);
#endif
