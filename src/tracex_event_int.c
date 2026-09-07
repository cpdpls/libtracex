#include <stdlib.h>
#include <string.h>
#include "tracex_event_int.h"


void tracex_destroy_event(struct tracex_event_entry_t **entry)
{
    if (entry != NULL)
    {
        if (*entry != NULL)
        {
            tracex_list_delete(&(*entry)->node);
            (*entry)->node.next = NULL;
            (*entry)->node.prev = NULL;

            free (*entry);
            *entry = NULL;
        }
    }
}

void tracex_destroy_event_list(struct tracex_list *head)
{
    struct tracex_event_entry_t *entry;
    struct tracex_event_entry_t *next;
    
    if (head != NULL)
    {
        tracex_list_for_each_entry_safe(entry, next, head, node)
        {
            tracex_destroy_event(&entry);
        }

    }
}