#ifndef __TRACE_PARSER_CTRL_H__
#define __TRACE_PARSER_CTRL_H__

#include <stdlib.h>
#include "trace_ctrl.h"



int trace_parse_header(struct trace_control_header *control_header, void *trace_buffer);

#endif