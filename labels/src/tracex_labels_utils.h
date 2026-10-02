#ifndef __TRACEX_LABELS_UTILS_H__
#define __TRACEX_LABELS_UTILS_H__

#include <stdint.h>
#include "tracex/tracex_labels_errno.h"
#include "cJSON.h"

tracex_labels_ret_t tracex_labels_utils_load_json_file(const uint8_t *path, cJSON **root);
#endif