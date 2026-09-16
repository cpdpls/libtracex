#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "tracex_errno.h"
#include "tracex_core.h"
#include "tracex_event_int.h"
#include "tracex_obj_int.h"
#include "tracex_utils.h"

static struct tracex_list created_handlers_list;    /* List of the current handles created */
static uint8_t tracex_init_done = 0;                /* Flag set to detect wether tracex has been init */

void tracex_init(void)
{
    /* First try to detect the endianess of the host system */
    tracex_utils_detect_indianess();

    /*  Init the global list of created handlers */
    if (created_handlers_list.next == NULL || created_handlers_list.prev == NULL)
    {
        tracex_list_init(&created_handlers_list);
    }

    tracex_init_done = 1;

}

/*TODO: Since we are using a global variable for the list of created handlers, let's also use a mutex for when creating new onces */
tracex_ret_t tracex_create_new_handler(tracex_handler_t **new_handler)
{
    struct tracex_handler *handler;
    tracex_ret_t status;


    /* Check if tracex has been initialized correctly */
    if (!tracex_init_done)
    {
        status = TRACEX_NOT_INIT;
        goto handle_exit;
    }

    if (new_handler == NULL)
    {
        status = TRACEX_BAD_INPUT_PTR;
        goto handle_exit;
    }
    handler = (struct tracex_handler*)malloc(sizeof(struct tracex_handler));

    if (handler == NULL)
    {
        status = TRACEX_ALLOC_FAILURE;
        goto handle_exit;
    }

    /* Faster to memset the structure to 0 */
    memset(handler, 0, sizeof(struct tracex_handler));

    /* Init the different contexts parts */
    if ((status = tracex_header_int_init(&handler->hdr_ctx)) != TRACEX_SUCCESS)
    {
        status = TRACEX_INIT_FAILURE;
        goto handle_error;
    }

    if ((status = tracex_object_int_init(&handler->objs_ctx)) != TRACEX_SUCCESS)
    {
        status = TRACEX_INIT_FAILURE;
        goto handle_error;
    }

    if ((status = tracex_event_int_init(&handler->event_ctx)) != TRACEX_SUCCESS)
    {
        status = TRACEX_INIT_FAILURE;
        goto handle_error;
    }

    /* Insert the newly created handler inside the list */
    tracex_list_insert(&handler->node, &created_handlers_list);

    *new_handler = handler;
    status = TRACEX_SUCCESS;
    goto handle_exit;

handle_error:
    free(handler);
    handler = NULL;

handle_exit:

    return status;
}

tracex_ret_t tracex_register_callbacks(tracex_handler_t *handler, struct tracex_callbacks *callbacks)
{
    if (callbacks == NULL || handler == NULL)
    {
        return TRACEX_BAD_INPUT_PTR;
    }

    if (tracex_is_handler_valid(handler) != TRACEX_SUCCESS)
    {
        return TRACEX_INVALID_HANDLER;
    }

    handler->hdr_ctx.user_callback = callbacks->on_header_parsed;
    handler->objs_ctx.user_callback = callbacks->on_object_parsed;
    handler->event_ctx.user_callback = callbacks->on_event_parsed;

    return TRACEX_SUCCESS;

}

void tracex_destroy_handler(tracex_handler_t **handler)
{
    if (handler != NULL)
    {
        if (*handler != NULL)
        {
            if (tracex_is_handler_valid(*handler) == TRACEX_SUCCESS)
            {
                tracex_object_int_destroy_list(&(*handler)->objs_ctx);
                tracex_event_int_destroy_list(&(*handler)->event_ctx);
                tracex_list_delete(&(*handler)->node);
                memset((*handler), 0, sizeof(struct tracex_handler));
                free(*handler);
                *handler = NULL;
            }
        }
    }

}

tracex_ret_t tracex_parse(struct tracex_handler *handler, void *buffer, size_t buffer_length)
{
    tracex_ret_t status;
    uint64_t consumed;


    struct tracex_header_context *hdr_ctx;
    consumed = 0;

    status = TRACEX_NEED_MORE;

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

    hdr_ctx = &handler->hdr_ctx;

    if (hdr_ctx->header_parsed && !hdr_ctx->header_valid)
    {
        status = TRACEX_HEADER_NOT_VALID;
        goto handle_exit;
    }

    /* Are we in the beginning of the parsing, AKA parsing the header ? */
    if (handler->state == E_HEADER_PHASE)
    {
        status = tracex_header_int_parse(hdr_ctx, buffer, buffer_length, &consumed);

        if (status == TRACEX_SUCCESS)
        {
            /* Increment the total bytes processed from here */
            handler->raw_bytes_count += consumed;

            /* Assign the needed missing values for the next phases */
            handler->objs_ctx.name_size = handler->hdr_ctx.header.obj_registry_name_size;
            handler->objs_ctx.registry_size = handler->hdr_ctx.obj_registry_size;
            handler->event_ctx.registry_size = handler->hdr_ctx.event_registry_size;

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
        status = tracex_object_int_parse(&handler->objs_ctx, buffer, buffer_length, &consumed);

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
        status = tracex_event_int_parse(&handler->event_ctx, buffer, buffer_length, &consumed);

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

tracex_ret_t tracex_is_handler_valid(struct tracex_handler *handler)
{
    struct tracex_handler *iter;

    tracex_list_for_each_entry(iter, &created_handlers_list, node)
    {
        if (iter == handler)
        {
            return TRACEX_SUCCESS;
        }
    }

    return TRACEX_INVALID_HANDLER;
}


tracex_ret_t tracex_object_iterator_init(tracex_handler *handler, TRACEX_object_iterator_t **iterator)
{

    if (handler == NULL || iterator == NULL)
    {
        return TRACEX_BAD_INPUT_PTR;
    }

    if (tracex_is_handler_valid(handler) != TRACEX_SUCCESS)
    {
        return TRACEX_INVALID_HANDLER;
    }

    return tracex_object_int_iterator_init(&handler->objs_ctx, iterator);

}

tracex_ret_t tracex_object_iterator_next(TRACEX_object_iterator_t *iterator, const struct tracex_object **object)
{
    if (iterator == NULL || object == NULL)
    {
        return TRACEX_BAD_INPUT_PTR;
    }

    return tracex_object_int_iterator_next(iterator, object);
}

void tracex_object_iterator_end(TRACEX_object_iterator_t **iterator)
{
    if (iterator != NULL)
    {
        tracex_object_int_iterator_end(iterator);
    }

}