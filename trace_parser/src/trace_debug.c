#include <stddef.h>

#include "trace_debug.h"

void trace_debug_print_header(struct trace_control_header *control_header)
{
    if (control_header == NULL)
    {
        perror("TRACE_HEADER_DEBUG : Invalid header pointer !");
    }
    else
    {
        printf("----TRACE_HEADER_DEBUG----\n\n");
        printf("Header ID --> %d\n", control_header->header_id);
        printf("Header timer_valid_mask --> %d\n", control_header->header_timer_valid_mask);
        printf("Header trace base address --> 0x%08x\n", control_header->header_trace_base_addr);
        printf("Header registry start pointer --> 0x%08x\n", control_header->header_obj_registry_start_ptr);
        printf("Header reserved 1 --> %hu\n", control_header->header_res1);
        printf("Header registry name size --> %hu\n", control_header->header_obj_registry_name_size);
        printf("Header registry end pointer --> 0x%08x\n", control_header->header_obj_registry_end_ptr);
        printf("Header trace start pointer --> 0x%08x\n", control_header->header_buffer_start_ptr);
        printf("Header trace end pointer --> 0x%08x\n", control_header->header_buffer_end_ptr);
        printf("Header trace current pointer --> 0x%08x\n", control_header->header_buffer_current_ptr);
        printf("Header reserved 2 --> %d\n", control_header->header_res2);
        printf("Header reserved 3 --> %d\n", control_header->header_res3);
        printf("Header reserved 4 --> %d\n\n\n", control_header->header_res4);
    }
}