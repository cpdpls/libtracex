#ifndef __TRACE_PARSER_CTRL_H__
#define __TRACE_PARSER_CTRL_H__

#include <stdlib.h>
#include "trace_ctrl.h"
#include "trace_parser.h"

#define TRACE_PARSER_CTRL_LE                    4
#define TRACE_PARSER_CTRL_BE                    5

int trace_parser_ctrl_header(FILE *file_ptr, struct trace_control_header **parser);
void trace_parser_ctrl_destroy(struct trace_control_header *control_header);
#endif

