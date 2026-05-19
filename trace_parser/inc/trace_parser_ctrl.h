#ifndef __TRACE_PARSER_CTRL_H__
#define __TRACE_PARSER_CTRL_H__

#include <stdlib.h>
#include "trace_ctrl.h"

#define TRACE_PARSER_CTRL_INVALID_PTR           1
#define TRACE_PARSER_CTRL_INVALID_ID            2
#define TRACE_PARSER_CTRL_MEM_ERROR             3
#define TRACE_PARSER_CTRL_LE                    4
#define TRACE_PARSER_CTRL_BE                    5

int trace_parse_ctrl_header(void *trace_raw_buffer, struct trace_control_header **control_header);
void trace_parse_ctrl_destroy(struct trace_control_header *control_header);
int trace_get_endianess(struct trace_control_header *control_header);
#endif

