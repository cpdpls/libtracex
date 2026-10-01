#ifndef __TRACEX_HEADER_INT_H__
#define __TRACEX_HEADER_INT_H__

#include <stdint.h>

#include "tracex/tracex_header.h"
#include "tracex_obj_int.h"
#include "event.h"

#define TRACEX_HEADER_ID_BIG_ENDIAN     0x54585442
#define TRACEX_HEADER_ID_LITTLE_ENDIAN  0x42545854


struct tracex_header_context
{
    void (*on_header_parsed)(void *cb_data, struct tracex_header *header, tracex_ret_t status);      /* Callback to use when the header has been parsed */
    void                            *cb_data;
    struct tracex_header            header;                 /* User tracex header */
    uint8_t                         header_parsed;          /* Flag set when the header has been parsed */
    uint8_t                         header_valid;           /* Flag set when the header is valid */
    uint8_t                         byte_offset;            /* Byte offset in the header of the dump when parsing incrementally */
    enum tracex_dump_endianess      endianess;              /* Endianess of the raw dump */
    uint64_t                        obj_registry_size;      /* Total number of possible objects in the object registry */
    uint64_t                        event_registry_size;    /* Total number of possible objects in the object registry */
};

tracex_ret_t tracex_header_int_check_parsed(struct tracex_header_context *ctx);
tracex_ret_t tracex_header_int_check_valid(struct tracex_header_context *ctx);
tracex_ret_t tracex_header_int_get(struct tracex_header_context *ctx, struct tracex_header **header);
tracex_ret_t tracex_header_int_parse(struct tracex_header_context *ctx, void *buffer, size_t buff_len, uint64_t *consumed);
#endif