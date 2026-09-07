#include <stdlib.h>
#include <string.h>
#include "tracex_obj_int.h"

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