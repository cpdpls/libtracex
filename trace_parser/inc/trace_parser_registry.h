#ifndef __TRACE_PARSER_REGISTRY_H__
#define __TRACE_PARSER_REGISTRY_H__

#include <stdio.h>
#include "trace_ctrl.h"
#include "trace_registry.h"

int trace_parse_registry(FILE *file_ptr, struct trace_control_header *header, struct trace_registry **registry);
void trace_registry_destroy(struct trace_registry **registry);
#endif
