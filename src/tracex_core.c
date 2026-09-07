#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "tracex_errno.h"
#include "tracex_core.h"
#include "tracex_event_int.h"
#include "tracex_obj_int.h"

static struct tracex_list handlers_list;         /* List of the current handles created */

struct TRACEX_handler_t *TRACEX_createHandler(void)
{
    struct TRACEX_handler_t *handler;

    handler = (struct TRACEX_handler_t*)malloc(sizeof(struct TRACEX_handler_t));

    if (handler == NULL)
    {
        return NULL;
    }

    /* Faster to memset the structure to 0 */
    memset(handler, 0, sizeof(struct TRACEX_handler_t));

    /* Init the different mutexes */
    pthread_mutex_init(&handler->header_mutex, NULL);
    pthread_mutex_init(&handler->event_mutex, NULL);
    pthread_mutex_init(&handler->object_mutex, NULL);

    /* If this is the very first created handler, init the list */
    if (handlers_list.next == NULL || handlers_list.prev == NULL)
    {
        tracex_list_init(&handlers_list);
    }

    /* Init other lists inside the newly allocated handler */
    tracex_list_init(&handler->structured_raw.obj_list);
    tracex_list_init(&handler->structured_raw.event_list);
    tracex_list_insert(&handler->node, &handlers_list);


    return handler;
}

void TRACEX_destroyHandler(struct TRACEX_handler_t **handler)
{
    struct tracex_event_entry_t *iter_event;
    if (handler != NULL)
    {
        if (*handler != NULL)
        {
            tracex_destroy_event_list(&(*handler)->structured_raw.event_list);
            tracex_destroy_object_list(&(*handler)->structured_raw.obj_list);
            tracex_list_delete(&(*handler)->node);
            memset((*handler), 0, sizeof(struct TRACEX_handler_t));
            free(*handler);
            *handler = NULL;
        }
    }

}

TRACEX_Ret_t TRACEX_parse(struct TRACEX_handler_t *handler, void *buffer, size_t buffer_length)
{
    TRACEX_Ret_t status;
    uint64_t consumed;
    struct tracex_hdr_entry_t *hdr_ptr;

    if (handler == NULL || buffer == NULL)
    {
        return TRACEX_BAD_INPUT_PTR;
    }

    if (buffer_length == 0)
    {
        return TRACEX_NULL_LENGTH;
    }

    hdr_ptr = &handler->structured_raw.header;

    if (hdr_ptr->is_header_processed && !hdr_ptr->is_header_valid)
    {
        return TRACEX_HEADER_NOT_VALID;
    }

    if (handler->state == E_HEADER_PHASE)
    {
        status = tracex_parse_data(&handler->structured_raw.header, buffer, buffer_length, &consumed);

        if (status == TRACEX_SUCCESS)
        {
            handler->state = E_OBJECT_PHASE;
        }
        if (status == TRACEX_NEED_MORE)
        {

        }
        else
        {
            handler->header_valid = 0;

        }
    }

    handler->raw_bytes_count += buffer_length;

    return status;
}

TRACEX_Ret_t tracex_is_handler_valid(struct TRACEX_handler_t *handler)
{
    struct TRACEX_handler_t *iter;

    tracex_list_for_each_entry(iter, &handlers_list, node)
    {
        if (iter == handler)
        {
            return TRACEX_SUCCESS;
        }
    }

    return TRACEX_INVALID_HANDLER;
}