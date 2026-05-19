#ifndef __TRACE_CTRL_HEADER_H__
#define __TRACE_CTRL_HEADER_H__

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#define TRACE_MAGIC_NUMBER_ID_LE    0x54585442
#define TRACE_MAGIC_NUMBER_ID_BE    0x42545854


struct trace_control_header
{
    uint32_t header_id;
    uint32_t header_timer_valid_mask;
    uint32_t header_trace_base_addr;
    uint32_t header_obj_registry_start_ptr;
    uint16_t header_res1;
    uint16_t header_obj_registry_name_size;
    uint32_t header_obj_registry_end_ptr;
    uint32_t header_buffer_start_ptr;
    uint32_t header_buffer_end_ptr;
    uint32_t header_buffer_current_ptr;
    uint32_t header_res2;
    uint32_t header_res3;
    uint32_t header_res4;
} __attribute__((packed));

#endif
