#ifndef TRACEX_LABELS_UTILS_H
#define TRACEX_LABELS_UTILS_H

#include <stdint.h>
#include "cJSON.h"

int tracex_resolver_utils_load_json_file(const uint8_t *path, cJSON **root);
#endif