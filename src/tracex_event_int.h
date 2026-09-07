#ifndef __TRACEX_EVENT_INT_H__
#define __TRACEX_EVENT_INT_H__

#include <stdint.h>
#include "tracex_list.h"

struct tracex_event_int_t
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
    struct tracex_event_int_t event;
    struct tracex_list node;
};

void tracex_destroy_event(struct tracex_event_entry_t **entry);
void tracex_destroy_event_list(struct tracex_list *head);
#endif