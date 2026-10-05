#ifndef TRACEX_HEADER_INT_H
#define TRACEX_HEADER_INT_H

#include <stdint.h>

#include "tracex/tracex_header.h"
#include "tracex_obj_int.h"

#define TRACEX_HEADER_ID_BIG_ENDIAN     0x54585442
#define TRACEX_HEADER_ID_LITTLE_ENDIAN  0x42545854

struct tracex_header_raw
{
    uint32_t id;
    uint32_t timestamp_mask;
    uint32_t trace_base_addr;
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


struct tracex_header_context
{
    void (*on_header_parsed)(void *cb_data, struct tracex_header *header, tracex_ret_t status);      /* Callback to use when the header has been parsed */
    void                            *cb_data;
    struct tracex_header_raw        staging_raw_header;     /* Saved staging raw header when parsing incrementally  */
    uint8_t                         staging_raw_offset;     /* Saved staging raw header offset when parsing incrementally */
    struct tracex_header            *user_header;           /* Allocated User header */
    uint8_t                         header_parsed;          /* Flag set when the header has been parsed */
    uint8_t                         header_valid;           /* Flag set when the header is valid */
    enum tracex_dump_endianess      endianess;              /* Endianess of the raw dump */
    uint64_t                        obj_registry_size;      /* Total number of possible objects in the object registry */
    uint64_t                        event_registry_size;    /* Total number of possible objects in the object registry */
};

void tracex_header_destroy_context(struct tracex_header_context *ctx);
tracex_ret_t tracex_header_int_check_parsed(struct tracex_header_context *ctx);
tracex_ret_t tracex_header_int_check_valid(struct tracex_header_context *ctx);
tracex_ret_t tracex_header_int_get(struct tracex_header_context *ctx, struct tracex_header **header);
tracex_ret_t tracex_header_int_parse(struct tracex_header_context *ctx, void *buffer, size_t buff_len, uint64_t *consumed);

#endif
