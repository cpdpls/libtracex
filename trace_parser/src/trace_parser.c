#include "trace_parser_ctrl.h"




struct trace_parser *trace_parser_open(uint8_t *trace_path)
{

}
void trace_parser_destroy(struct trace_parser *trace_parser);





static inline int trace_parse_magic_number(void *trace_buffer);

static inline int trace_parse_magic_number(void *trace_buffer)
{
    int magic_status;

    if (*(uint32_t*)trace_buffer != TRACE_MAGIC_NUMBER_ID_LE &&
        *(uint32_t*)trace_buffer != TRACE_MAGIC_NUMBER_ID_BE)
    {
        magic_status =  TRACE_INVALID_ID;
    }
    else
    {
        magic_status = 0;
    }
    return magic_status;
}
int trace_parse_header(struct trace_control_header *control_header, void *trace_buffer)
{

    if (control_header == NULL || trace_buffer == NULL)
        return TRACE_INVALID_PTR;
    
    if (trace_parse_magic_number(trace_buffer) != 0)
        return TRACE_INVALID_ID;

    *control_header = *(struct trace_control_header *)trace_buffer;

    return 0;
    
}