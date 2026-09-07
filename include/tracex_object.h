#ifndef __TRACEX_OBJECT_H__
#define __TRACEX_OBJECT_H__

#include <stdint.h>
#include "tracex_errno.h"

typedef struct TRACEX_handler_t TRACEX_handler_t;   /* Forward declaration */

enum TRACEX_object_type_t
{
    E_TEST_F,

};
enum TRACEX_object_param1
{
    E_TEST_G,

};
enum TRACEX_object_param2
{
    E_TEST_H,

};


typedef struct TRACEX_object_t
{
    uint8_t                     available;          /* Flag set when the object is available */
    enum TRACEX_object_type_t   type;               /* The type of the object */
    uint32_t                    object_pointer;     /* Object pointer address in the dump */
    enum TRACEX_object_param1   param1;             /* Parameter 1 of the object */
    enum TRACEX_object_param2   param2;             /* Parameter 2 of the object */
    uint8_t                     *object_name;       /* Object name */
} TRACEX_object_t;

typedef struct TRACEX_object_list_t
{
    uint32_t                count;
    struct TRACEX_object_t  **objects;
} TRACEX_object_list_t;


/**
 * @brief Returns the parsed objects from the previous parsing.
 * 
 * @param handler Pointer to the previously allocated handler
 * @param object_list Pointer location where the objects list will be returned to.
 * @return TRACEX_Ret_t TRACEX_SUCCESS on success, TRACEX_NEED_MORE if more bytes are required,
 *          Other value from TRACEX_Ret_t otherwise.
 * 
 * TODO: Make this function thread safe by using a mutex and pause the parser before returning the objects
 */

TRACEX_Ret_t TRACEX_getObjects(struct TRACEX_handler_t *handler, TRACEX_object_list_t *object_list);
#endif