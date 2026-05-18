#ifndef __TRACE_PARSER_H__
#define __TRACE_PARSER_H__

#include "trace_ctrl.h"

#define TRACE_PARSER_MEM_ERR                    1
#define TRACE_PARSER_INVALID_PTR                2
#define TRACE_PARSER_TRACE_FILE_NOT_FOUND       3
#define TRACE_PARSER_EMPTY_TRACE_FILE           4
#define TRACE_PARSER_INVALID_TRACE_SIZE         5
#define TRACE_PARSER_INVALID_MAGIC_NUMBER       6
#define TRACE_PARSER_CORRUPTED_CTRL_HEADER      7

enum trace_endianess 
{
    TRACE_PARSER_LE,
    TRACE_PARSER_BE,
};

struct trace_dump
{
    struct trace_control_header *control_header;
};

struct trace_parser
{
    enum trace_endianess endianess;
    struct trace_dump dump;
    FILE *trace_file;
    size_t trace_size;
    void *trace_buffer;
};

struct trace_parser *trace_parser_open(uint8_t *trace_path);
void trace_parser_destroy(struct trace_parser *trace_parser);
#endif