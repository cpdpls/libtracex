#ifndef __TRACEX_OBJ_STR_H__
#define __TRACEX_OBJ_STR_H__

#include <stdint.h>
#include "tracex_object.h"
#include "tracex_list.h"

#define TRACXEX_OBJ_TYPE_RESERVED_SIZE 6u

struct tracex_object_params_str
{
    const uint8_t *param1_label;                /* String for the param1 */
    const uint8_t *param2_label;                /* String for the param2 */
};

struct tracex_object_labels {
    uint8_t *object_type_str;                   /* The object type string */
    enum tracex_object_type object_id;          /* The object ID as found in the enum tracex_object_type */
    struct tracex_object_params_str params_str; /* The object strings for the param 1 and param 2 */

    tracex_node node;                           /* The next node */
};


const uint8_t *tracex_object_type_to_str(enum tracex_object_type type);
struct tracex_object_params_str tracex_object_param_to_str(enum tracex_object_type type);
void tracex_object_labels_add(struct tracex_object_labels *new_labels);

void tracex_object_destroy_labels(void);
#endif