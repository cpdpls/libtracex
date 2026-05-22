#ifndef __TRACE_PARSER_EVENT_H__
#define __TRACE_PARSER_EVENT_H__

#include <stdio.h>
#include "trace_ctrl.h"
#include "trace_events.h"

int trace_parse_event_registry(FILE *file_ptr, struct trace_control_header *header, struct trace_events_registry **event_registry);
void trace_event_registry_destroy(struct trace_events_registry **event_registry);

#endif
