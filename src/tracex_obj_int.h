#ifndef __TRACEX_OBJ_INT_H__
#define __TRACEX_OBJ_INT_H__

#include <stdint.h>
#include "tracex_list.h"
#include "tracex_object.h"

struct tracex_object_raw_t
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
    struct tracex_object_raw_t      obj;        /* Raw parsed object from the raw dump */
    struct TRACEX_object_t          user_obj;   /* User parsed object */
    struct tracex_list              node;       /* Next object node */
};

struct tracex_object_dump_t
{
    struct tracex_list          obj_list;               /* List of parsed objects */
    struct TRACEX_object_t      **objects;              /* User list of objects */
    uint64_t                    object_count;           /* Total count of objects */
    uint64_t                    curr_obj_byte_count;    /* Saved current object byte count when parsing incrementally */
    pthread_mutex_t             object_mutex;           /* Mutex used when retrieving and parsing objects */
};

void tracex_destroy_object(struct tracex_object_entry_t **object);
void tracex_destroy_object_list(struct tracex_list *head);
#endif