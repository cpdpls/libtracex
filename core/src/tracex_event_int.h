#ifndef __TRACEX_EVENT_INT_H__
#define __TRACEX_EVENT_INT_H__

#include <stdint.h>
#include "tracex/tracex_event.h"
#include "tracex/tracex_errno.h"
#include "tracex_list.h"

struct tracex_event_raw
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
    struct tracex_event     event;             /* Parsed event from the raw dump */
    tracex_node             node;              /* Next event node */
};

struct tracex_event_context
{
    void (*on_event_parsed)(void *cb_data, struct tracex_event *event, tracex_ret_t status);    /* Callback to use when a new event had been parsed */
    void                        *cb_data;
    struct tracex_list          event_list;         /* List of parsed events */
    uint64_t                    curr_count;         /* Total count of events for the current parssing session */
    struct tracex_event_raw     staging_raw_event;  /* Staging raw event for the incremental parsing */
    uint16_t                    staging_raw_offset; /* Staging raw event offset when parsing incrementally */
    struct tracex_event_entry   *tmp_event;         /* Temp allocated event object */
    uint64_t tot_count;                             /* Total count of events */
    uint64_t                    registry_size;      /* Total number of posssible events in the event registry */

};

/**
 * @brief Initializes an event context structure 
 * 
 * @param ctx pointer the context to be initialized
 * @return tracex_ret_t
 */
tracex_ret_t tracex_event_int_init(struct tracex_event_context *ctx);

/**
 * @brief Computes the total size of an event buffer, that is the maximum of events entry the buffer holds 
 * 
 * @param registry_size pointer to where the total entry of events is saved
 * @param start The start address of the event buffer
 * @param stop The end address of the event buffer
 * @return tracex_ret_t 
 */
tracex_ret_t tracex_event_int_compute_registry_size(uint64_t *registry_size, uint32_t start, uint32_t stop);

/**
 * @brief Parses events as much as possible from a given raw buffer of bytes
 * 
 * @param ctx holds the context of the event parser
 * @param buffer raw buffer pointer containing bytes to be parsed to a set of events
 * @param buff_len the size of the provided buffer pointer
 * @param consumed number of consumed bytes from the raw buffer after the function call
 * @return tracex_ret_t 
 */
tracex_ret_t tracex_event_int_parse(struct tracex_event_context *ctx, void *buffer, size_t buff_len, uint64_t *consumed);

void tracex_event_int_destroy_context(struct tracex_event_context *ctx);
#endif