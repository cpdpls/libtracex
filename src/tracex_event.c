#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "tracex_event_int.h"
#include "tracex_event.h"
#include "tracex_errno.h"
#include "tracex_core.h"

TRACEX_Ret_t TRACEX_getEvents(struct TRACEX_handler_t *handler, TRACEX_event_list_t *event_list)
{
    if (handler == NULL || event_list == NULL)
    {
        return TRACEX_BAD_INPUT_PTR;
    }

    /* Check the validity of the handler */
    if (tracex_is_handler_valid(handler) != TRACEX_SUCCESS)
    {
        return TRACEX_INVALID_HANDLER;
    }

    pthread_mutex_lock(&handler->event_mutex);
    if (handler->structured_raw.event_count == 0)
    {
        /* Release the mutex before returning to the caller */
        pthread_mutex_unlock(&handler->event_mutex);
        return TRACEX_NEED_MORE;
    }

    
    event_list->count = handler->structured_raw.event_count;
    event_list->events = handler->user_dump.events;

    /* Release the mutex before returning to the caller */
    pthread_mutex_unlock(&handler->event_mutex);
    
    return TRACEX_SUCCESS;
}

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