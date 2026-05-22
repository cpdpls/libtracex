#ifndef __TRACE_HEADER_H__
#define __TRACE_HEADER_H__

#include <stdint.h>
#include "trace_errno.h"
#include "trace_io.h"

typedef struct trace_header_int trace_header_int;

enum trace_header_endianess
{
    HEADER_LITTLE_ENDIAN,
    HEADER_BIG_ENDIAN,
};

struct trace_header
{
    unsigned int timer_mask;
    unsigned int objs_registry_offset;
    unsigned int events_registry_offset;
    unsigned int objs_registry_size;
    unsigned int events_registry_size;
    unsigned int obj_name_size;
    enum trace_header_endianess endianess;

    trace_header_int *header_int;
};

/**
 * @brief Allocates memory for the header structure
 *
 * @param header_ptr returned allocated pointer
 * @return TRACERet_t function call result
 */
TRACERet_t trace_create_header(struct trace_header **header_ptr);

/**
 * @brief Unallocate the header pointer
 *
 * @param header_ptr pointer of pointer to the header to be detroyed
 * @return void
 */
void trace_destroy_header(struct trace_header **header_ptr);

/**
 * @brief Parses a header from the supplied buffer pointer
 *
 * @param header_ptr pointer to a header structure to be filled in
 * @param raw_buffer Buffer from which to parse the header
 * @return TRACERet_t return code
 */
TRACERet_t trace_parse_header(struct trace_header *header_ptr, struct trace_io_dev *io_dev);
#endif