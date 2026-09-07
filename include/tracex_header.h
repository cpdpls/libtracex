#ifndef __TRACEX_HEADER_H__
#define __TRACEX_HEADER_H__

#include <stdint.h>
#include "tracex_errno.h"

typedef struct TRACEX_handler_t TRACEX_handler_t;

enum TRACEX_dump_endianess_t
{
    E_TRACEX_LITTLE_ENDIAN,
    E_TRACEX_BIG_ENDIAN,
};

struct TRACEX_header_t
{
    uint32_t timer_mask;
    enum TRACEX_dump_endianess_t endianess;
    uint8_t object_name_size;
};

TRACEX_Ret_t TRACEX_checkHeaderValid(void *buffer);
TRACEX_Ret_t TRACEX_getTimerMask(struct TRACEX_handler_t *parser, uint32_t *mask);

#endif