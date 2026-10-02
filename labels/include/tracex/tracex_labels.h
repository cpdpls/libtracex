#ifndef __TRACEX_LABELS_H__
#define __TRACEX_LABELS_H__

#include <stdint.h>
#include "tracex/tracex_labels_errno.h"

struct tracex_labels_event_infos_labels {
    const uint8_t *info1_label;
    const uint8_t *info2_label;
    const uint8_t *info3_label;
    const uint8_t *info4_label;

};

struct tracex_labels_object_params_labels
{
    const uint8_t *param1_label;                /* String for the param1 */
    const uint8_t *param2_label;                /* String for the param2 */
};


tracex_labels_ret_t tracex_labels_event_load_labels(uint8_t *json_path);
tracex_labels_ret_t tracex_labels_object_load_labels(uint8_t *json_path);

const uint8_t *tracex_labels_event_id_to_str(uint32_t id);
struct tracex_labels_event_infos_labels tracex_labels_event_infos_to_str(uint32_t id);

const uint8_t *tracex_labels_object_type_to_str(uint32_t type);
struct tracex_labels_object_params_labels tracex_labels_object_param_to_str(uint32_t type);


#endif