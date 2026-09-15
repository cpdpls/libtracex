#ifndef __TRACEX_EVENT_INT_H__
#define __TRACEX_EVENT_INT_H__

#include <stdint.h>
#include "tracex_list.h"
#include "tracex_event.h"

struct tracex_event_raw_t
{
    uint32_t     tx_trace_buffer_entry_thread_pointer;
    uint32_t     tx_trace_buffer_entry_thread_priority;
    uint32_t     tx_trace_buffer_entry_event_id;
    uint32_t     tx_trace_buffer_entry_time_stamp;
    uint32_t     tx_trace_buffer_entry_information_field_1;
    uint32_t     tx_trace_buffer_entry_information_field_2;
    uint32_t     tx_trace_buffer_entry_information_field_3;
    uint32_t     tx_trace_buffer_entry_information_field_4;

}__attribute__((__packed__));

struct tracex_event_entry_t
{
    TRACEX_event_t              event;                  /* Parsed event from the raw dump */
    struct tracex_list          node;                   /* Next event node */
};

struct tracex_event_dump_t
{
    void (*parserCallback)(TRACEX_event_t *event, TRACEX_Ret_t status);    /* Callback to use when a new event had been parsed */
    struct tracex_list          event_list;             /* List of parsed events */
    uint64_t                    curr_event_count;       /* Total count of events for the current parssing */
    struct tracex_event_raw_t   current_raw_event;      /* Saved current event when parsing incrementally */
    uint64_t                    tot_event_count;        /* Total count of events */
    uint64_t                    curr_event_offset;      /* Saved current events byte count when parsing incrementally */
    pthread_mutex_t             event_mutex;            /* Mutex used when retrieving and parsing events */
    struct tracex_header_dump_t *hdr_dump_ptr;          /* Pointer to the header the events belongs too. Used in order to get the event trace buffer length */
};

uint64_t tracex_event_compute_registry_size(uint32_t start, uint32_t stop);
TRACEX_Ret_t tracex_event_check_registry_valid(uint32_t start, uint32_t stop);
TRACEX_Ret_t tracex_init_events(struct tracex_event_dump_t *event_dump, struct tracex_header_dump_t *hdr_dump);
TRACEX_Ret_t tracex_parse_events(struct tracex_event_dump_t *event_dump, void *buffer, size_t buff_len, uint64_t *consumed);
void tracex_destroy_event(struct tracex_event_entry_t **entry);
void tracex_destroy_event_list(struct tracex_event_dump_t *event_dump);
#endif