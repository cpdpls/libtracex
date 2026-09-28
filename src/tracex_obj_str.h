#ifndef __TRACEX_OBJ_STR_H__
#define __TRACEX_OBJ_STR_H__

#include <stdint.h>
#include "tracex_object.h"


struct tracex_object_params_labels
{
    const uint8_t *param1_label;                /* String for the param1 */
    const uint8_t *param2_label;                /* String for the param2 */
};

const uint8_t *tracex_object_type_to_str(enum tracex_object_type type);
struct tracex_object_params_labels tracex_object_param_to_str(enum tracex_object_type type);

tracex_ret_t tracex_object_load_labels(uint8_t *json_path);
void tracex_object_destroy_labels(void);
#endif