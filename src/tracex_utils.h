#ifndef __TRACEX_UTILS_H__
#define __TRACEX_UTILS_H__

#include "tracex_errno.h"

void tracex_utils_detect_indianess(void);
tracex_ret_t tracex_utils_load_json_file(const uint8_t *path, cJSON **root);
#endif