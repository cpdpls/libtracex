#ifndef __TRACEX_OBJ_INT_H__
#define __TRACEX_OBJ_INT_H__

#include <stdint.h>
#include "tracex_list.h"

struct tracex_object_int_t
{
    uint8_t  obj_available;
    uint8_t  obj_type;
    uint8_t  obj_res1;
    uint8_t  obj_res2;
    uint32_t obj_ptr;
    uint32_t obj_param_1;
    uint32_t obj_param_2;
    uint8_t  *obj_name;

    struct tracex_list node;
};

struct tracex_object_entry_t
{
    struct tracex_object_int_t obj;
    struct tracex_list node;
};

void tracex_destroy_object(struct tracex_object_entry_t **object);
void tracex_destroy_object_list(struct tracex_list *head);
#endif