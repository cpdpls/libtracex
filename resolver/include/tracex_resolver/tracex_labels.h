#ifndef TRACEX_LABELS_H
#define TRACEX_LABELS_H

#include <stdint.h>
#include "tracex/tracex_object.h"
#include "tracex/tracex_event.h"


int tracex_resolver_event_load_labels(const char *json_path);
int tracex_resolver_object_load_labels(const char *json_path);

struct tracex_event_labels tracex_resolver_get_event_labels(uint32_t event_id);
struct tracex_object_labels tracex_resolver_get_object_labels(uint32_t type);

void tracex_resolver_event_destroy_labels(void);
void tracex_resolver_object_destroy_labels(void);

#endif