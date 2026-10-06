#ifndef LABELS_ENGINE_UTILS_H
#define LABELS_ENGINE_UTILS_H

#include <stdint.h>
#include "cJSON.h"

int labels_engine_utils_load_json_file(const uint8_t *path, cJSON **root);
#endif