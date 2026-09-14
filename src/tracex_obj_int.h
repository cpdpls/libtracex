#ifndef __TRACEX_OBJ_INT_H__
#define __TRACEX_OBJ_INT_H__

#include <stdint.h>
#include "tracex_list.h"
#include "tracex_object.h"
#include "tracex_header_int.h"
#include "tracex_errno.h"

enum tracex_obj_fsm_t
{
    E_OBJ_PARSE_OTHERS,
    E_OBJ_PARSE_NAME,
    
};

struct tracex_obj_raw_t
{
    uint8_t  obj_vailable;
    uint8_t  obj_type;
    uint8_t  obj_res1;
    uint8_t  obj_res2;
    uint32_t obj_pointer;
    uint32_t obj_param_1;
    uint32_t obj_param_2;
    uint8_t  *obj_name;

}__attribute__((__packed__));


struct tracex_object_entry_t
{
    TRACEX_object_t                 obj;        /* Parsed object from the raw dump */
    struct tracex_list              node;       /* Next object node */
};

struct tracex_object_dump_t
{
    uint8_t                         dump_init;              /* Flag set when the dump has been initialized */
    struct tracex_list              obj_list;               /* List of parsed objects */
    uint64_t                        tot_object_count;       /* Total count of objects */
    uint64_t                        curr_object_count;      /* Total count of objects for the current parsing */
    struct tracex_obj_raw_t         current_raw_obj;
    struct tracex_object_entry_t    *current_object;        /* Saved current object when parsing incrementally */
    uint8_t                         curr_obj_offset;        /* Saved current object byte count when parsing the fields incrementally */
    
    /* Because the objects structure has a variable object name that is not known
        until the header has been parser, we need to allocate the pointer *obj_name when creating the object.
        This makes it so that it is not possible to use memcpy on the whole structure when parsing
        incrementally, otherwise the pointer would get only the first 8 bytes of the string and so get an
        invalid value. Therefore we need to keep track wether we are parsing other fields and wether
        we are parsing the obj name field
    */
    enum tracex_obj_fsm_t           obj_fsm;                /* State machine used when parsing fields */
    pthread_mutex_t                 object_mutex;           /* Mutex used when retrieving and parsing objects */
    struct tracex_header_dump_t     *hdr_dump_ptr;         /* Pointer to the header the object belongs too. Used as parsing objects need it */
};

struct tracex_obj_iterator
{
    const TRACEX_object_t   **objects;
    size_t                  count;
    size_t                  index;
};

TRACEX_Ret_t tracex_object_check_registry_valid(uint32_t start, uint32_t stop, uint32_t obj_name_length);
uint64_t tracex_object_compute_total_objects(uint32_t start, uint32_t stop, uint32_t obj_name_length);
TRACEX_Ret_t tracex_init_objects(struct tracex_object_dump_t *obj_dump, struct tracex_header_dump_t *hdr_dump);
TRACEX_Ret_t tracex_parse_objects(struct tracex_object_dump_t *obj_dump, void *buffer, size_t buff_len, uint64_t *consumed);
void tracex_destroy_object(struct tracex_object_entry_t **object);
void tracex_destroy_object_list(struct tracex_list *head);

TRACEX_Ret_t tracex_object_iterator_init(struct tracex_object_dump_t *obj_dump, TRACEX_object_iterator_t **iter);
TRACEX_Ret_t tracex_object_iter_next(TRACEX_object_iterator_t *iter, const TRACEX_object_t **object);
void tracex_object_iter_end(TRACEX_object_iterator_t **iter);
#endif