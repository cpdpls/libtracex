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

};

struct tracex_event_entry_t
{
    struct tracex_event_raw_t      event;        /* Raw parsed event from the raw dump */
    struct TRACEX_event_t          user_event;   /* User parsed event */
    struct tracex_list             node;         /* Next event node */
};

struct tracex_event_dump_t
{
    struct tracex_list          event_list;             /* List of parsed events */
    struct TRACEX_object_t      **events;               /* User list of events */
    uint64_t                    event_count;            /* Total count of events */
    uint64_t                    curr_event_byte_count;  /* Saved current events byte count when parsing incrementally */
    pthread_mutex_t             event_mutex;            /* Mutex used when retrieving and parsing events */
};

void tracex_destroy_event(struct tracex_event_entry_t **entry);
void tracex_destroy_event_list(struct tracex_list *head);
#endif