#ifndef __TRACE_PARSER_H__
#define __TRACE_PARSER_H__

#include <stdio.h>
#include "trace_ctrl.h"
#include "trace_events.h"
#include "trace_objects_registry.h"


enum trace_endianess 
{
    TRACE_PARSER_LE,
    TRACE_PARSER_BE,
};

struct trace_parsed
{
    struct trace_control_header *header;
    struct trace_objects_registry *objects_registry;
    struct trace_events_registry *events_regitry;
};

struct trace_parser
{
    enum trace_endianess endianess;
    struct trace_parsed parsed_trace;
    FILE *trace_file;
    size_t trace_size;
};

int trace_parser_open(uint8_t *trace_path, struct trace_parser **parser_ptr);
int trace_parser_parse_data(struct trace_parser *parser_ptr);
void trace_parser_close(struct trace_parser *trace_parser);
#endif
