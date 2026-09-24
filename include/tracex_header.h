#ifndef __TRACEX_HEADER_H__
#define __TRACEX_HEADER_H__

#include <stdint.h>
#include "tracex_errno.h"

typedef struct tracex_handler tracex_handler;       /* Forward declaration */

enum tracex_dump_endianess
{
    E_TRACEX_LITTLE_ENDIAN,
    E_TRACEX_BIG_ENDIAN,
};


struct tracex_header
{
    uint32_t id;
    uint32_t timestamp_mask;
    uint32_t trace_base_add;
    uint32_t obj_registry_start_ptr;
    uint16_t res1;
    uint16_t obj_registry_name_size;
    uint32_t obj_registry_end_ptr;
    uint32_t event_buff_start_ptr;
    uint32_t event_buff_end_ptr;
    uint32_t event_buff_curr_ptr;
    uint32_t res2;
    uint32_t res3;
    uint32_t res4;
} __attribute__((__packed__));

#endif