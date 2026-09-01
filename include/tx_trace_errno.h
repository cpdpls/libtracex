#ifndef __TX_TRACE_ERRNO_H__
#define __TX_TRACE_ERRNO_H__

#include "abstractio/aio_errno.h"

typedef enum {
     TX_TRACE_SUCCES = AIO_SUCCESS,
     TX_TRACE_ALLOC_FAIL = AIO_ALLOC_FAIL,
     TX_TRACE_BAD_INPUT_PTR = AIO_BAD_INPUT_PTR,
     TX_GENERAL_FAILURE,
} TXTRACE_Ret_t;

#endif

