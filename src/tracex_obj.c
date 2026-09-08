#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "tracex_obj_int.h"
#include "tracex_errno.h"
#include "tracex_core.h"
#include "tracex_object.h"

// TRACEX_Ret_t TRACEX_getObjects(struct TRACEX_handler_t *handler, TRACEX_object_list_t *object_list)
// {
//     if (handler == NULL || object_list == NULL)
//     {
//         return TRACEX_BAD_INPUT_PTR;
//     }

//     /* Check the validity of the handler */
//     if (tracex_is_handler_valid(handler) != TRACEX_SUCCESS)
//     {
//         return TRACEX_INVALID_HANDLER;
//     }

//     /* Try to get the object mutex */
//     pthread_mutex_lock(&handler->object_mutex);

//     /* Check if the at least 1 object has been parsed */
//     if (handler->structured_raw.object_count == 0)
//     {
//         /* Release the mutex before returning to the caller */
//         pthread_mutex_unlock(&handler->object_mutex);
//         return TRACEX_NEED_MORE;
//     }
//     object_list->count = handler->structured_raw.object_count;
//     object_list->objects = handler->user_dump.objects;

//     /* Release the mutex before returning to the caller */
//     pthread_mutex_unlock(&handler->object_mutex);
//     return TRACEX_SUCCESS;
// }

void tracex_destroy_object(struct tracex_object_entry_t **object)
{
    if (object != NULL)
    {
        if (*object != NULL)
        {
            /* Free the object name */
            if ((*object)->obj.obj_name != NULL)
            {
                free((*object)->obj.obj_name);
                (*object)->obj.obj_name = NULL;
            }

            tracex_list_delete(&(*object)->node);
            (*object)->node.next = NULL;
            (*object)->node.prev = NULL;

            free(*object);
            (*object) = NULL;
        }
    }
}

void tracex_destroy_object_list(struct tracex_list *head)
{

    struct tracex_object_entry_t *entry;
    struct tracex_object_entry_t *next;
    
    if (head != NULL)
    {
        tracex_list_for_each_entry_safe(entry, next, head, node)
        {
            tracex_destroy_object(&entry);
        }
    }

}