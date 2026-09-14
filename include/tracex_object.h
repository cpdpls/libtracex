#ifndef __TRACEX_OBJECT_H__
#define __TRACEX_OBJECT_H__

#include <stdint.h>
#include "tracex_errno.h"

typedef struct TRACEX_handler_t TRACEX_handler_t;   /* Forward declaration */

enum TRACEX_ObjectType
{
    TRACEX_FF,
};

typedef struct
{
    uint8_t  vailable;
    enum TRACEX_ObjectType type;

    // uint8_t  type;
    uint8_t  res1;
    uint8_t  res2;
    uint32_t pointer;
    uint32_t param_1;
    uint32_t param_2;
    uint8_t  *name;

}TRACEX_object_t;


typedef struct tracex_obj_iterator TRACEX_object_iterator_t;

TRACEX_Ret_t TRACEX_objectIteratorInit(TRACEX_handler_t *handler, TRACEX_object_iterator_t **iterator);
TRACEX_Ret_t TRACEX_objectIteratorNext(TRACEX_object_iterator_t *iterator, const TRACEX_object_t **object);
void TRACEX_objectIteratorEnd(TRACEX_object_iterator_t **iter);
#endif