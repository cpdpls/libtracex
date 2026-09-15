#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "tracex_errno.h"
#include "tracex_core.h"
#include "tracex_event_int.h"
#include "tracex_obj_int.h"
#include "tracex_utils.h"

static struct tracex_list handlers_list;            /* List of the current handles created */
static uint8_t is_tracex_init = 0;                  /* Flag set to detect wether tracex has been init */

void TRACEX_INIT()
{
    /* First try to detect the endianess of the host system */
    tracex_utils_detect_indianess();

    /*  Init the global list of created handlers */
    if (handlers_list.next == NULL || handlers_list.prev == NULL)
    {
        tracex_list_init(&handlers_list);
    }

    is_tracex_init = 1;

}

TRACEX_Ret_t TRACEX_registerCallbacks(struct TRACEX_handler_t *handler, TRACEX_Callbacks_t *callbacks)
{
    if (callbacks == NULL || handler == NULL)
    {
        return TRACEX_BAD_INPUT_PTR;
    }

    if (tracex_is_handler_valid(handler) != TRACEX_SUCCESS)
    {
        return TRACEX_INVALID_HANDLER;
    }

    handler->raw_dump.header.parserCallback = callbacks->HeaderParsed;
    handler->raw_dump.events.parserCallback = callbacks->EventParsed;
    handler->raw_dump.objs.parserCallback = callbacks->ObjectParsed;

    return TRACEX_SUCCESS;

}

TRACEX_Ret_t TRACEX_createHandler(struct TRACEX_handler_t **handler_ptr)
{
    struct TRACEX_handler_t *handler;
    TRACEX_Ret_t status;


    /* Check if tracex has been initialized correctly */
    if (!is_tracex_init)
    {
        status = TRACEX_NOT_INIT;
        goto handle_exit;
    }

    if (handler_ptr == NULL)
    {
        status = TRACEX_BAD_INPUT_PTR;
        goto handle_exit;
    }
    handler = (struct TRACEX_handler_t*)malloc(sizeof(struct TRACEX_handler_t));

    if (handler == NULL)
    {
        status = TRACEX_ALLOC_FAILURE;
        goto handle_exit;
    }

    /* Faster to memset the structure to 0 */
    memset(handler, 0, sizeof(struct TRACEX_handler_t));

    /* Init the different dump parts */
    if ((status = tracex_init_header(&handler->raw_dump.header)) != TRACEX_SUCCESS)
    {
        status = TRACEX_INIT_FAILURE;
        goto handle_error;
    }

    if ((status = tracex_init_objects(&handler->raw_dump.objs, &handler->raw_dump.header)) != TRACEX_SUCCESS)
    {
        status = TRACEX_INIT_FAILURE;
        goto handle_error;
    }

    if ((status = tracex_init_events(&handler->raw_dump.events, &handler->raw_dump.header)) != TRACEX_SUCCESS)
    {
        status = TRACEX_INIT_FAILURE;
        goto handle_error;
    }

    /* Insert the newly created handler inside the list */
    tracex_list_insert(&handler->node, &handlers_list);

    *handler_ptr = handler;
    status = TRACEX_SUCCESS;
    goto handle_exit;

handle_error:
    free(handler);
    handler = NULL;

handle_exit:

    return status;
}

void TRACEX_destroyHandler(struct TRACEX_handler_t **handler)
{
    struct tracex_event_entry_t *iter_event;
    if (handler != NULL)
    {
        if (*handler != NULL)
        {
            if (tracex_is_handler_valid(*handler) == TRACEX_SUCCESS)
            {
                tracex_destroy_object_list(&(*handler)->raw_dump.objs);
                tracex_destroy_event_list(&(*handler)->raw_dump.events);
                tracex_list_delete(&(*handler)->node);
                memset((*handler), 0, sizeof(struct TRACEX_handler_t));
                free(*handler);
                *handler = NULL;
            }
        }
    }

}

TRACEX_Ret_t TRACEX_parse(struct TRACEX_handler_t *handler, void *buffer, size_t buffer_length)
{
    TRACEX_Ret_t status;
    uint64_t consumed;


    struct tracex_header_dump_t *hdr_dump_ptr;
    consumed = 0;

    if (handler == NULL || buffer == NULL)
    {
        status = TRACEX_BAD_INPUT_PTR;
        goto handle_exit;
    }

    if (buffer_length == 0)
    {
        status = TRACEX_NULL_LENGTH;
        goto handle_exit;
    }

    if (tracex_is_handler_valid(handler) != TRACEX_SUCCESS)
    {
        status = TRACEX_INVALID_HANDLER;
        goto handle_exit;
    }

    hdr_dump_ptr = &handler->raw_dump.header;

    if (hdr_dump_ptr->header_parsed && !hdr_dump_ptr->header_valid)
    {
        status = TRACEX_HEADER_NOT_VALID;
        goto handle_exit;
    }

    /* Are we in the beginning of the parsing, AKA parsing the header */
    if (handler->state == E_HEADER_PHASE)
    {
        status = tracex_parse_header(&handler->raw_dump.header, buffer, buffer_length, &consumed);

        if (status == TRACEX_SUCCESS)
        {
            /* Increment the total bytes processed from here */
            handler->raw_bytes_count += consumed;
            
            /* Go to the next phase, which is parsing the objects */
            handler->state = E_OBJECT_PHASE;

            /* If the remaining for the next phase is 0, ask for more */
            if (buffer_length - consumed == 0)
            {
                status = TRACEX_NEED_MORE;
                goto handle_exit;
            }
            /*  Enough bytes for the next phase, increment the buffer pointer 
            *   And substract the buffer length with the just consumed total
            */
            else
            {
                buffer += consumed;
                buffer_length -= consumed;
            }
        }
    }

    /* We are now trying to parse the objects */
    if (handler->state == E_OBJECT_PHASE)
    {
        status = tracex_parse_objects(&handler->raw_dump.objs, buffer, buffer_length, &consumed);

        if (status == TRACEX_SUCCESS)
        {
            handler->state = E_EVENT_PHASE;
        }
        
        handler->raw_bytes_count += consumed;

        /* If the remaining for the next phase is 0, ask for more */
        if (buffer_length - consumed == 0)
        {
            status = TRACEX_NEED_MORE;
            goto handle_exit;
        }
        /*  Enough bytes for the next phase, increment the buffer pointer 
        *   And substract the buffer length with the just consumed total
        */
        else
        {
            buffer += consumed;
            buffer_length -= consumed;
        }


    }
    
    if (handler->state == E_EVENT_PHASE)
    {
        status = tracex_parse_events(&handler->raw_dump.events, buffer, buffer_length, &consumed);

        if (status == TRACEX_SUCCESS)
        {
            handler->state = E_OBJECT_PHASE;
        }

        handler->raw_bytes_count += consumed;
        buffer += consumed;
        buffer_length -= consumed;
    }

handle_exit:
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

TRACEX_Ret_t TRACEX_getHeader(struct TRACEX_handler_t *handler, struct TRACEX_header_t **header)
{

    if (handler == NULL || header == NULL)
    {
        return TRACEX_BAD_INPUT_PTR;
    }

    if (tracex_is_handler_valid(handler) != TRACEX_SUCCESS)
    {
        return TRACEX_INVALID_HANDLER;
    }

    return tracex_header_get_header(&handler->raw_dump.header, header);

}

TRACEX_Ret_t TRACEX_isHeaderParsed(struct TRACEX_handler_t *handler)
{
    if (handler == NULL)
    {
        return TRACEX_BAD_INPUT_PTR;
    }

    if (tracex_is_handler_valid(handler) != TRACEX_SUCCESS)
    {
        return TRACEX_INVALID_HANDLER;
    }

    return tracex_is_header_parsed(&handler->raw_dump.header);
}

TRACEX_Ret_t TRACEX_isHeaderValid(struct TRACEX_handler_t *handler)
{
    if (handler == NULL)
    {
        return TRACEX_BAD_INPUT_PTR;
    }

    if (tracex_is_handler_valid(handler) != TRACEX_SUCCESS)
    {
        return TRACEX_INVALID_HANDLER;
    }

    return tracex_is_header_valid(&handler->raw_dump.header);
}

TRACEX_Ret_t TRACEX_objectIteratorInit(TRACEX_handler_t *handler, TRACEX_object_iterator_t **iterator)
{

    if (handler == NULL || iterator == NULL)
    {
        return TRACEX_BAD_INPUT_PTR;
    }

    if (tracex_is_handler_valid(handler) != TRACEX_SUCCESS)
    {
        return TRACEX_INVALID_HANDLER;
    }

    return tracex_object_iterator_init(&handler->raw_dump.objs, iterator);

}

TRACEX_Ret_t TRACEX_objectIteratorNext(TRACEX_object_iterator_t *iterator, const TRACEX_object_t **object)
{
    if (iterator == NULL || object == NULL)
    {
        return TRACEX_BAD_INPUT_PTR;
    }

    return tracex_object_iter_next(iterator, object);
}

void TRACEX_objectIteratorEnd(TRACEX_object_iterator_t **iter)
{
    if (iter != NULL)
    {
        tracex_object_iter_end(iter);
    }

}