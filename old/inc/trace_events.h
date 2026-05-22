#ifndef __TRACE_EVENTS_H__
#define __TRACE_EVENTS_H__

#include <stddef.h>
#include <stdint.h>

enum events_id
{
    THREAD_RESUME = 1,
    THREAD_SUSPEND = 2,
    ISR_ENTER = 3,
    ISR_EXIT = 4,
    TIME_SLICE = 5,
    RUNNING = 6,
};
struct trace_event_entry
{
    uint32_t thread_pointer;
    uint32_t thread_priority;
    uint32_t event_id;
    uint32_t time_stamp;
    uint32_t info_1;
    uint32_t info_2;
    uint32_t info_3;
    uint32_t info_4;
}__attribute__((packed));

struct trace_parsed_event_registry
{
    struct trace_event_entry **events;
    size_t events_count;
};

struct trace_events_registry
{
    struct trace_parsed_event_registry *parsed_registry;
    void *raw_buffer;
    size_t raw_buffer_size;
};
#endif

