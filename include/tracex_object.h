#ifndef __TRACEX_OBJECT_H__
#define __TRACEX_OBJECT_H__

#include <stdint.h>
#include "tracex_errno.h"

typedef struct tracex_handler tracex_handler;   /* Forward declaration */

typedef struct tracex_obj_iterator TRACEX_object_iterator_t;

struct tracex_object
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



tracex_ret_t tracex_object_iterator_init(tracex_handler *handler, TRACEX_object_iterator_t **iterator);
tracex_ret_t tracex_object_iterator_next(TRACEX_object_iterator_t *iterator, const struct tracex_object **object);
void tracex_object_iterator_end(TRACEX_object_iterator_t **iterator);
#endif