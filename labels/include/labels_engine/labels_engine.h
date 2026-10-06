#ifndef LABELS_ENGINE_H
#define LABELS_ENGINE_H

#include <stdint.h>
#include "tracex/tracex_resolver.h"



int labels_engine_event_load_labels(const char *json_path);
int labels_engine_object_load_labels(const char *json_path);

tracex_resolver_labels labels_engine_resolve_labels(enum tracex_resolver_request_type request, uint32_t label_id);

void labels_engine_event_destroy_labels(void);
void labels_engine_object_destroy_labels(void);

#endif