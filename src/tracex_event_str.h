#ifndef __TRACEX_EVENT_STR_H__
#define __TRACEX_EVENT_STR_H__

#include <stdint.h>
#include "tracex_errno.h"

#define TRACEX_EVENT_DEFAULT_JSON_PATH "data/events.json"

struct tracex_event_infos_labels {
    const uint8_t *info1_label;
    const uint8_t *info2_label;
    const uint8_t *info3_label;
    const uint8_t *info4_label;

};

const uint8_t *tracex_event_id_to_str(uint32_t id);
struct tracex_event_infos_labels tracex_event_infos_to_str(uint32_t id);

tracex_ret_t tracex_event_load_labels(uint8_t *json_path);
void tracex_event_destroy_labels(void);
#endif