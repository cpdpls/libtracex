#include "trace_parser_ctrl.h"
#include "trace_ctrl.h"
#include <stdio.h>

static inline int trace_parse_magic_number(void *trace_buffer);
static inline int trace_parse_read_header(FILE *trace_file, void *trace_buffer);

static inline int trace_parse_magic_number(void *trace_buffer)
{
    int magic_status;

    if (*(uint32_t*)trace_buffer != TRACE_MAGIC_NUMBER_ID_LE &&
        *(uint32_t*)trace_buffer != TRACE_MAGIC_NUMBER_ID_BE)
    {
        magic_status =  TRACE_PARSER_CTRL_INVALID_ID;
    }
    else
    {
        magic_status = 0;
    }
    return magic_status;
}

static inline int trace_parse_read_header(FILE *trace_file, void *trace_buffer)
{
    if (fread(trace_buffer, 1, sizeof(struct trace_control_header), trace_file) != sizeof(struct trace_control_header))
    {
        return TRACE_PARSER_CTRL_MEM_ERROR;
    }

}
int trace_parse_ctrl_header(FILE *trace_file, void *trace_raw_buffer, struct trace_control_header **control_header)
{
    int result;

    if (control_header == NULL || trace_raw_buffer == NULL || trace_file == NULL)
        return TRACE_PARSER_CTRL_INVALID_PTR;
    
    trace_raw_buffer = (void*)malloc(sizeof(struct trace_control_header));
    if (trace_raw_buffer == NULL)
    {
        return TRACE_PARSER_CTRL_MEM_ERROR;
    }

    result = trace_parse_read_header(trace_file, trace_raw_buffer); 

    if (trace_parse_magic_number(trace_buffer) != 0)
        return TRACE_PARSER_CTRL_INVALID_ID;

    *control_header = *(struct trace_control_header *)trace_buffer;

error_cleanup_all:

    
    return 0;
    
}
void trace_parse_ctrl_destroy(struct trace_control_header *control_header)
{

}
int trace_get_endianess(struct trace_control_header *control_header)
{
    if(control_header->header_id == TRACE_MAGIC_NUMBER_ID_BE)
        return TRACE_PARSER_CTRL_BE;
    if (control_header->header_id == TRACE_MAGIC_NUMBER_ID_LE)
        return TRACE_PARSER_CTRL_LE;

    else
        return TRACE_PARSER_CTRL_INVALID_ID;

}
