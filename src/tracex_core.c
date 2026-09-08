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

    /* TODO: Maybe init the list here ? */
    is_tracex_init = 1;

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
    
    if (tracex_init_header(&handler->raw_dump.header) != TRACEX_SUCCESS)
    {
        status = TRACEX_INIT_FAILURE;
        goto handle_error;
    }

    /* TODO: Init the objects and events here */

    /* If this is the very first created handler, init the list */
    if (handlers_list.next == NULL || handlers_list.prev == NULL)
    {
        tracex_list_init(&handlers_list);
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
                /*TODO: Destroy the objects and events dumps */
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

    if (handler == NULL || buffer == NULL)
    {
        return TRACEX_BAD_INPUT_PTR;
    }

    if (buffer_length == 0)
    {
        return TRACEX_NULL_LENGTH;
    }

    if (tracex_is_handler_valid(handler) != TRACEX_SUCCESS)
    {
        return TRACEX_INVALID_HANDLER;
    }

    /*TODO: Check if the handler is valid by calling the checker function */

    hdr_dump_ptr = &handler->raw_dump.header;

    if (hdr_dump_ptr->header_parsed && !hdr_dump_ptr->header_valid)
    {
        return TRACEX_HEADER_NOT_VALID;
    }

    if (handler->state == E_HEADER_PHASE)
    {
        status = tracex_parse_header(&handler->raw_dump.header, buffer, buffer_length, &consumed);

        if (status == TRACEX_SUCCESS)
        {
            handler->state = E_OBJECT_PHASE;
        }
        if (status == TRACEX_NEED_MORE)
        {

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