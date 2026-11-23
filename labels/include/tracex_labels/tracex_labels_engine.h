#ifndef TRACEX_LABELS_ENGINE_H
#define TRACEX_LABELS_ENGINE_H

#include <stdint.h>
#include "tracex/tracex_resolver.h"



int tracex_labels_engine_event_load_labels(const char *json_path);
int tracex_labels_engine_object_load_labels(const char *json_path);

tracex_resolver_labels tracex_labels_resolve_labels(enum tracex_resolver_request_type request, uint32_t label_id);

void tracex_labels_engine_event_destroy(void);
void tracex_labels_engine_object_destroy(void);

#endif