#ifndef __TRACE_PARSER_REGISTRY_H__
#define __TRACE_PARSER_REGISTRY_H__

#include <stdio.h>
#include "trace_ctrl.h"
#include "trace_objects_registry.h"

int trace_parse_object_registry(FILE *file_ptr, struct trace_control_header *header, struct trace_objects_registry **object_registry);
char  *trace_parse_object_get_type(unsigned int object_index, struct trace_objects_registry *object_registry);
void trace_object_registry_destroy(struct trace_objects_registry **object_registry);
#endif
