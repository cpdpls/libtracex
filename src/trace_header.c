#include <stdlib.h>
#include "trace_header.h"
#include "trace_io_dev.h"

/**
 * @brief Checks if the header is a TraceX file dump
 *
 * @return int TRACERet_t function call result
 */
// static TRACERet_t is_tracex_type()

struct trace_header_int
{
    uint32_t id;
    uint32_t timer_valid_mask;
    uint32_t base_addr;
    uint32_t obj_start_ptr;
    uint16_t res1;
    uint16_t obj_name_size;
    uint32_t obj_registry_end_ptr;
    uint32_t events_start_ptr;
    uint32_t events_end_ptr;
    uint32_t events_current_ptr;
    uint32_t res2;
    uint32_t res3;
    uint32_t res4;
} __attribute__((packed));

TRACERet_t trace_create_header(struct trace_header **header_ptr)
{
    struct trace_header *tmp_header;
    struct trace_header_int *tmp_header_int;
    TRACERet_t op_result;

    /* Sanitize the user input */
    if (header_ptr == NULL)
    {
        op_result = TRACE_BAD_INPUT_PTR;
        goto return_operation_result;
    }

    /* Allocate a user header */
    tmp_header = (struct trace_header*)malloc(sizeof(struct trace_header));
    if (tmp_header == NULL)
    {
        op_result = TRACE_ALLOC_FAIL;
        goto return_operation_result;
    }

    /* Allocated an internal header for parsing later */
    tmp_header_int = (struct trace_header_int *)malloc(sizeof(struct trace_header_int));

    /* Deallocate the previous header structure */
    if (tmp_header_int == NULL)
    {
        op_result = TRACE_ALLOC_FAIL;
        goto cleanup_failed_allocation;
    }

    /* Assign the internal header inside the user header */
    tmp_header->header_int = tmp_header_int;

    return TRACE_SUCCESS;


cleanup_failed_allocation:
    trace_destroy_header(&tmp_header);

return_operation_result:
    return op_result;
}

void trace_destroy_header(struct trace_header **header_ptr)
{
    if (header_ptr != NULL)
    {
        if (*header_ptr != NULL)
        {
            if ((*header_ptr)->header_int != NULL)
            {
                free((*header_ptr)->header_int);
                (*header_ptr)->header_int = NULL;
            }
            free(*header_ptr);
            *header_ptr = NULL;
        }
    }
}
TRACERet_t trace_parse_header(struct trace_header *header_ptr, struct trace_io_dev *io_dev)
{
    if (io_dev == NULL || header_ptr == NULL)
        return TRACE_BAD_INPUT_PTR;

    //*header_ptr = *(struct trace_header *)raw_buffer;

    return 0;
}