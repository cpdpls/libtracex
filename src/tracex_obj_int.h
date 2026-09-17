#ifndef __TRACEX_OBJ_INT_H__
#define __TRACEX_OBJ_INT_H__

#include <stdint.h>
#include "tracex_list.h"
#include "tracex_object.h"
#include "tracex_header_int.h"
#include "tracex_errno.h"


enum tracex_obj_fsm
{
    E_OBJ_PARSE_OTHERS,
    E_OBJ_PARSE_NAME,    
};


struct tracex_object_int
{
    uint8_t  available;
    uint8_t  type;
    uint8_t  res1;
    uint8_t  res2;
    uint32_t pointer;
    uint32_t param_1;
    uint32_t param_2;
    uint8_t  *name;

}__attribute__((__packed__));


struct tracex_object_entry
{
    struct tracex_object_int        raw_obj;        /* Parsed object from the raw dump */
    struct tracex_object            usr_obj;        /* Parsed user object */
    tracex_node                     node;           /* Next object node */
};

struct tracex_object_context
{
    void (*user_callback)(struct tracex_object *object, tracex_ret_t status);   /* Callback to use when a new object has been parsed */
    struct tracex_list              obj_list;                                   /* List of parsed objects */
    uint64_t                        curr_count;                                 /* Total count of objects for the current parsing */
    struct tracex_object_entry      *current_entry;                             /* Saved current object when parsing incrementally */
    uint8_t                         curr_offset;                                /* Saved current object byte count when parsing the fields incrementally */
    uint64_t                        tot_count;                                  /* Total count of objects */
    pthread_mutex_t                 mutex;                                      /* Mutex used when retrieving and parsing objects */
    uint64_t                        registry_size;                              /* Total number of possible objects in the object registry */
    uint16_t                        name_size;                                  /* Max object name length of an object */

    /* Because the objects structure has a variable object name that is not known
        until the header has been parser, we need to allocate the pointer *obj_name when creating the object.
        This makes it so that it is not possible to use memcpy on the whole structure when parsing
        incrementally, otherwise the pointer would get only the first 8 bytes of the string and so get an
        invalid value. Therefore we need to keep track wether we are parsing other fields and wether
        we are parsing the obj name field
    */
    enum tracex_obj_fsm             fsm;                                        /* State machine used when parsing fields */
};

struct tracex_obj_iterator
{
    const struct tracex_object   **objects;
    size_t                      count;
    size_t                      index;
};

tracex_ret_t tracex_object_int_init(struct tracex_object_context *ctx);
tracex_ret_t tracex_object_int_compute_registry_size(uint64_t *registry_size, uint32_t start, uint32_t stop, uint32_t name_size);
tracex_ret_t tracex_object_int_parse(struct tracex_object_context *ctx, void *buffer, size_t buff_len, uint64_t *consumed);
void tracex_object_int_destroy_list(struct tracex_object_context *ctx);

tracex_ret_t tracex_object_int_iterator_init(struct tracex_object_context *ctx, TRACEX_object_iterator_t **iterator);
tracex_ret_t tracex_object_int_iterator_next(TRACEX_object_iterator_t *iterator, const struct tracex_object **object);
void tracex_object_int_iterator_end(TRACEX_object_iterator_t **iterator);
#endif