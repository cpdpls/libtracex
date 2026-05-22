#ifndef __TRACE_DEBUG_H__
#define __TRACE_DEBUG_H__

#include "trace_ctrl.h"
#include "trace_parser.h"

void trace_parser_debug_print_header(struct trace_parsed_control_header*control_header);
void trace_parser_debug_parser(struct trace_parser *parser);

void trace_parser_debug_print_used_objects(struct trace_parser *parser);
void trace_parser_debug_print_object(struct trace_parser *parser, int obj_index);
void trace_parser_debug_print_all_objects(struct trace_parser *parser);
void trace_parser_printf_summary(struct trace_parser *parser);
void trace_parser_print_list_used_object(struct trace_parser *parser);

#endif
