#ifndef __TRACEX_EVENT_INT_H__
#define __TRACEX_EVENT_INT_H__

#include <stdint.h>
#include "tracex_list.h"
#include "tracex_event.h"
#include "tracex_errno.h"

struct tracex_event_entry
{
    struct tracex_event     event;                  /* Parsed event from the raw dump */
    struct tracex_list      node;                   /* Next event node */
};

struct tracex_event_array_block
{
    struct tracex_event_entry *entries;
    struct tracex_list      node;

};

struct tracex_event_context
{
    void (*user_callback)(struct tracex_event *event, tracex_ret_t status);    /* Callback to use when a new event had been parsed */
    struct tracex_list              event_list;         /* List of parsed events */
    struct tracex_list              array_block_list;   /* List of allocated arrays of events */
    uint64_t                        cycle_count;        /* Total count of events for the current session */
    uint64_t                        cycle_valid_count;  /* Actual total count of events that are valid for the current */
    struct tracex_event_array_block *current_block;     /* Current working ptr to a block of event entries */
    struct tracex_event_entry       *curr_entry;        /* Saved current event when parsing incrementally */
    uint64_t                        curr_offset;        /* Saved current events byte count when parsing incrementally */
    uint64_t                        total_events;       /* Total count of events */
    pthread_mutex_t                 mutex;              /* Mutex used when retrieving and parsing events */
    uint64_t                        registry_size;      /* Total number of posssible events in the event registry */

};

tracex_ret_t tracex_event_int_init(struct tracex_event_context *ctx);
tracex_ret_t tracex_event_int_compute_registry_size(uint64_t *registry_size, uint32_t start, uint32_t stop);
tracex_ret_t tracex_event_int_parse(struct tracex_event_context *ctx, void *buffer, size_t buff_len, uint64_t *consumed);
void tracex_event_int_destroy_list(struct tracex_event_context *ctx);
#endif