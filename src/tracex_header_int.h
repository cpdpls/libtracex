#ifndef __TRACEX_HEADER_INT_H__
#define __TRACEX_HEADER_INT_H__

#include <stdint.h>
#include <pthread.h>
#include "tracex_header.h"

#define TRACEX_HEADER_ID_BIG_ENDIAN     0x54585442
#define TRACEX_HEADER_ID_LITTLE_ENDIAN  0x42545854

struct tracex_raw_header_t
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


struct tracex_header_dump_t
{
    void (*parserCallback)(struct TRACEX_header_t *header, TRACEX_Ret_t status);      /* Callback to use when the header has been parsed */
    struct TRACEX_header_t      user_hdr;               /* User tracex header */
    struct tracex_raw_header_t  raw_hdr;                /* Actual header as it is in the dump */
    pthread_mutex_t             header_mutex;           /* Mutex used when retrieving and parsing objects */
    uint8_t                     header_parsed;          /* Flag set when the header has been parsed */
    uint8_t                     header_valid;           /* Flag set when the header is valid */
    uint8_t                     byte_offset;            /* Byte offset in the header of the dump when parsing incrementally */
    uint64_t                    object_registry_size;   /* Total number of possible objects in the object registry */
    uint64_t                    event_registry_size;    /* Total number of posssible events in the event registry */
};

TRACEX_Ret_t tracex_init_header(struct tracex_header_dump_t *hdr_dump);
TRACEX_Ret_t tracex_is_header_parsed(struct tracex_header_dump_t *hdr_dump);
TRACEX_Ret_t tracex_is_header_valid(struct tracex_header_dump_t *hdr_dump);
TRACEX_Ret_t tracex_header_get_header(struct tracex_header_dump_t *hdr_dump, struct TRACEX_header_t **header);
TRACEX_Ret_t tracex_parse_header(struct tracex_header_dump_t *hdr_dump, void *buffer, size_t buff_len, uint64_t *consumed);
#endif