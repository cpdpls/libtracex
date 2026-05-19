#ifndef __TRACE_PARSER_H__
#define __TRACE_PARSER_H__

#include "trace_ctrl.h"

#define TRACE_PARSER_MEM_ERR                    1
#define TRACE_PARSER_INVALID_PTR                2
#define TRACE_PARSER_FILE_OP_ERROR              3
#define TRACE_PARSER_TRACE_FILE_NOT_FOUND       4
#define TRACE_PARSER_EMPTY_TRACE_FILE           5
#define TRACE_PARSER_FILE_TOO_BIG               6
#define TRACE_PARSER_INVALID_TRACE_SIZE         7
#define TRACE_PARSER_INVALID_MAGIC_NUMBER       8
#define TRACE_PARSER_CORRUPTED_CTRL_HEADER      9

enum trace_endianess 
{
    TRACE_PARSER_LE,
    TRACE_PARSER_BE,
};

struct trace_parsed
{
    struct trace_control_header *control_header;
};

struct trace_parser
{
    enum trace_endianess endianess;
    struct trace_parsed parsed_trace;
    FILE *trace_file;
    size_t trace_size;
    void *trace_raw_buffer;
};

int trace_parser_open(uint8_t *trace_path, struct trace_parser **parser_ptr);
void trace_parser_destroy(struct trace_parser *trace_parser);
#endif
