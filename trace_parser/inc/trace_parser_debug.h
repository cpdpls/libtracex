#ifndef __TRACE_DEBUG_H__
#define __TRACE_DEBUG_H__

#include "trace_parser.h"

void trace_parser_debug_print_header(struct trace_control_header *control_header);
void trace_parser_debug_parser(struct trace_parser *parser);

#endif
