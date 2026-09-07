#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <pthread.h>

#include "tracex_core.h"
#include "tracex_errno.h"

static TRACEX_Ret_t tracex_header_process(struct tracex_hdr_entry_t *hdr_entry);
static TRACEX_Ret_t is_header_valid(struct tracex_hdr_entry_t *hdr_entry);

TRACEX_Ret_t TRACEX_getHeader(struct TRACEX_handler_t *handler, struct TRACEX_header_t **header)
{
    if (handler == NULL || header == NULL)
    {
        return TRACEX_BAD_INPUT_PTR;
    }

    /* Check the validity of the handler */
    if (tracex_is_handler_valid(handler) != TRACEX_SUCCESS)
    {
        return TRACEX_INVALID_HANDLER;
    }

    /* Try to get the object mutex */
    pthread_mutex_lock(&handler->header_mutex);

    /* Check if the header has already been parsed */
    if (!handler->header_parsed)
    {
        /* Release the mutex before returning to the caller */
        pthread_mutex_unlock(&handler->header_mutex);
        return TRACEX_NEED_MORE;
    }

    /* Check if the header has been marked as invalid */
    if (handler->header_parsed && !handler->header_valid)
    {
        /* Release the mutex before returning to the caller */
        pthread_mutex_unlock(&handler->header_mutex);
        return TRACEX_HEADER_NOT_VALID;
    }

    /* Header is valid, return it to the user */
    *header = &handler->user_dump.header;

    /* Release the mutex before returning to the caller */
    pthread_mutex_unlock(&handler->header_mutex);

    return TRACEX_SUCCESS;
}

TRACEX_Ret_t tracex_parse_data(struct TRACEX_handler_t *handler, void *buffer, size_t buff_len, uint64_t *consumed)
{
    TRACEX_Ret_t status;
    size_t bytes_to_copy;
    struct tracex_hdr_entry_t *hdr_entry;

    
    if (handler == NULL || buffer == NULL)
    {
        status =  TRACEX_BAD_INPUT_PTR;
        goto handle_error;
    }

    pthread_mutex_lock(&handler->header_mutex);

    hdr_entry = &handler->structured_raw.header;

    if (hdr_entry->byte_offset >= sizeof(struct tracex_hdr_int_t))
    {
        status = TRACEX_HEADER_BAD_OFFSET_START;
        goto handle_release_and_exit;
    }

    if (buff_len + hdr_entry->byte_offset >= sizeof(struct tracex_hdr_int_t))
    {
        bytes_to_copy = sizeof(struct tracex_hdr_int_t) - hdr_entry->byte_offset;
    }
    else
    {
        bytes_to_copy = buff_len;

    }

    memcpy(((void*)&hdr_entry->hdr)+ hdr_entry->byte_offset, buffer, bytes_to_copy);

    *consumed = bytes_to_copy;
    hdr_entry->byte_offset += bytes_to_copy;


    if (hdr_entry->byte_offset >= sizeof(struct tracex_hdr_int_t))
    {
        hdr_entry->is_header_processed = 1;
        status = TRACEX_SUCCESS;
    }

    else
    {
        status = TRACEX_NEED_MORE;
    }

    goto handle_release_and_exit;

    /* We should have now have parsed a full header */



    
handle_error:
    hdr_entry->is_header_processed = 1;
    *consumed = 0;
    goto handle_exit;

handle_release_and_exit:
    /* Release the mutex before returning to the caller */
    pthread_mutex_unlock(&handler->header_mutex);

handle_exit:
    return status;

}

static TRACEX_Ret_t tracex_header_process(struct tracex_hdr_entry_t *hdr_entry)
{
    TRACEX_Ret_t status;



}