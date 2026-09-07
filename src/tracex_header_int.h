#ifndef __TRACEX_HEADER_INT_H__
#define __TRACEX_HEADER_INT_H__

#include <stdint.h>

struct tracex_hdr_int_t
{
    uint32_t id;
    uint32_t timer_valid_mask;
    uint32_t trace_base_address;
    uint32_t obj_registry_start_pointer;
    uint16_t res1;
    uint16_t obj_registry_name_size;
    uint32_t obj_registry_end_pointer;
    uint32_t buff_start_pointer;
    uint32_t buff_end_pointer;
    uint32_t buff_current_pointer;
    uint32_t res2;
    uint32_t res3;
    uint32_t res4;
} __attribute__((__packed__));


struct tracex_hdr_entry_t
{
    struct tracex_hdr_int_t hdr;
    uint8_t is_header_processed;
    uint8_t is_header_valid;
    uint8_t byte_offset;
};


TRACEX_Ret_t tracex_parse_data(struct TRACEX_handler_t *handler, void *buffer, size_t buff_len, uint64_t *consumed);
#endif