#ifndef TRACEX_OBJ_INT_H
#define TRACEX_OBJ_INT_H

#include <stdint.h>
#include "tracex/tracex_object.h"
#include "tracex/tracex_errno.h"
#include "tracex_list.h"
#include "tracex_header_int.h"


enum tracex_obj_fsm
{
    E_OBJ_PARSE_OTHERS,
    E_OBJ_PARSE_NAME    
};


struct tracex_object_raw
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
    struct tracex_object            obj;        /* Parsed user object */
    tracex_node                     node;           /* Next object node */
};

struct tracex_object_context
{
    void (*on_object_parsed)(void *cb_data, struct tracex_object *object, tracex_ret_t status);   /* Callback to use when a new object has been parsed */
    void                            *cb_data;
    struct tracex_list              obj_list;               /* List of parsed objects */
    uint64_t                        curr_count;             /* Total count of objects for the current parsing */
    struct tracex_object_raw        staging_raw_obj;        /* Staging raw object for the incremental parsing */
    uint8_t                         staging_raw_offset;     /* Staging raw object offset when parsing incrementally */
    struct tracex_object_entry      *tmp_obj;               /* Temp allocated object */
    uint64_t                        tot_count;              /* Total count of objects */
    uint64_t                        registry_size;          /* Total number of possible objects in the object registry */
    uint16_t                        name_size;              /* Max object name length of an object */

    /* Because the objects structure has a variable object name that is not known
        until the header has been parser, we need to allocate the pointer *obj_name when creating the object.
        This makes it so that it is not possible to use memcpy on the whole structure when parsing
        incrementally, otherwise the pointer would get only the first 8 bytes of the string and so get an
        invalid value. Therefore we need to keep track wether we are parsing other fields and wether
        we are parsing the obj name field
    */
    enum tracex_obj_fsm             fsm;                                        /* State machine used when parsing fields */
    struct tracex_resolver_obj_engine resolver_engine;
};

struct tracex_obj_iterator
{
    const struct tracex_object   **objects;
    size_t                      count;
    size_t                      index;
};

tracex_ret_t tracex_object_int_init(struct tracex_object_context *ctx);
tracex_ret_t tracex_object_int_compute_registry_size(uint64_t *registry_size, uint32_t start, uint32_t stop, uint32_t name_size);
tracex_ret_t tracex_object_int_register_resolver_engine(struct tracex_object_context *ctx, struct tracex_resolver_obj_engine *engine, int *init_status);
tracex_ret_t tracex_object_int_parse(struct tracex_object_context *ctx, void *buffer, size_t buff_len,uint64_t *consumed);
void tracex_object_destroy_context(struct tracex_object_context *ctx);

tracex_ret_t tracex_object_int_iterator_init(struct tracex_object_context *ctx, TRACEX_object_iterator_t **iterator);
tracex_ret_t tracex_object_int_iterator_next(TRACEX_object_iterator_t *iterator, const struct tracex_object **object);
void tracex_object_int_iterator_end(TRACEX_object_iterator_t **iterator);
#endif
