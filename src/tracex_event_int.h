#ifndef __TRACEX_EVENT_INT_H__
#define __TRACEX_EVENT_INT_H__

#include <stdint.h>
#include "tracex_list.h"
#include "tracex_event.h"
#include "tracex_errno.h"

struct tracex_event_int
{
    uint32_t    thread_pointer;     /* Thread pointer when the event happened*/
    uint32_t    thread_priority;    /* Thread priority */
    uint32_t    event_id;           /* Event ID of the event */
    uint32_t    time_stamp;         /* Timestamp when the event happened*/
    uint32_t    info1;              /* Info 1 of the event */
    uint32_t    info2;              /* Info 2 of the event */
    uint32_t    info3;              /* Info 3 of the event */
    uint32_t    info4;              /* Info 4 of the event */
} __attribute__((__packed__));

struct tracex_event_entry
{
    struct tracex_event_int raw_event;
    struct tracex_event     user_event;             /* Parsed event from the raw dump */
    tracex_node             node;                   /* Next event node */
};

struct tracex_event_context
{
    void (*on_event_parsed)(void *cb_data, struct tracex_event *event, tracex_ret_t status);    /* Callback to use when a new event had been parsed */
    void                        *cb_data;
    struct tracex_list          event_list;     /* List of parsed events */
    uint64_t                    curr_count;     /* Total count of events for the current parssing session */
    struct tracex_event_entry   *curr_entry;    /* Saved current event when parsing incrementally */
    uint64_t                    curr_offset;    /* Saved current events byte count when parsing incrementally */
    uint64_t                    tot_count;      /* Total count of events */
    uint64_t                    registry_size;  /* Total number of posssible events in the event registry */

};

tracex_ret_t tracex_event_int_init(struct tracex_event_context *ctx);
tracex_ret_t tracex_event_int_compute_registry_size(uint64_t *registry_size, uint32_t start, uint32_t stop);
tracex_ret_t tracex_event_int_parse(struct tracex_event_context *ctx, void *buffer, size_t buff_len, uint64_t *consumed);
void tracex_event_destroy_context(struct tracex_event_context *ctx);
#endif