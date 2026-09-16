#ifndef __TRACEX_HEADER_INT_H__
#define __TRACEX_HEADER_INT_H__

#include <stdint.h>
#include <pthread.h>

#include "tracex_header.h"
#include "tracex_obj_int.h"
#include "tracex_event_int.h"

#define TRACEX_HEADER_ID_BIG_ENDIAN     0x54585442
#define TRACEX_HEADER_ID_LITTLE_ENDIAN  0x42545854


struct tracex_header_context
{
    void (*user_callback)(struct tracex_header *header, tracex_ret_t status);      /* Callback to use when the header has been parsed */
    struct tracex_header            header;                 /* User tracex header */
    pthread_mutex_t                 header_mutex;           /* Mutex used when retrieving and parsing objects */
    uint8_t                         header_parsed;          /* Flag set when the header has been parsed */
    uint8_t                         header_valid;           /* Flag set when the header is valid */
    uint8_t                         byte_offset;            /* Byte offset in the header of the dump when parsing incrementally */
    enum tracex_dump_endianess      endianess;              /* Endianess of the raw dump */
    struct tracex_object_context    *obj_entry;             /* Pointer to the associated object entry */
    struct tracex_event_context     *event_entry;           /* Pointer to the associated event entry */
};

tracex_ret_t tracex_header_init(struct tracex_header_context *ctx);
tracex_ret_t tracex_header_check_parsed(struct tracex_header_context *ctx);
tracex_ret_t tracex_header_check_valid(struct tracex_header_context *ctx);
tracex_ret_t tracex_header_get(struct tracex_header_context *ctx, struct tracex_header **header);
tracex_ret_t tracex_header_parse(struct tracex_header_context *ctx, void *buffer, size_t buff_len, uint64_t *consumed);
#endif