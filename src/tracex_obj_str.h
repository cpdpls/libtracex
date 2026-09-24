#ifndef __TRACEX_OBJ_STR_H__
#define __TRACEX_OBJ_STR_H__

#include <stdint.h>
#include "tracex_object.h"

#define TRACXEX_OBJ_TYPE_RESERVED_SIZE 6u

struct tracex_object_params_str
{
    const uint8_t *param1_label;
    const uint8_t *param2_label;
};

const uint8_t *tracex_object_type_to_str(enum tracex_object_type type);
struct tracex_object_params_str tracex_object_param_to_str(enum tracex_object_type type);
#endif