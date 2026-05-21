#ifndef __TRACE_DEBUG_H__
#define __TRACE_DEBUG_H__

#include "trace_ctrl.h"
#include "trace_parser.h"

void trace_parser_debug_print_header(struct trace_parsed_control_header*control_header);
void trace_parser_debug_parser(struct trace_parser *parser);

void trace_parser_debug_print_objects(struct trace_parser *parser);

#endif
