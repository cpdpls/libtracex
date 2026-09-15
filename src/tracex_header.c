#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <pthread.h>

#include "tracex_core.h"
#include "tracex_errno.h"
#include "tracex_obj_int.h"

static TRACEX_Ret_t process_header(struct tracex_header_dump_t *hdr_dump);
static TRACEX_Ret_t parse_incrementally(struct tracex_header_dump_t *hdr_dump, void *buffer, size_t buff_len, uint64_t *consumed);

TRACEX_Ret_t tracex_init_header(struct tracex_header_dump_t *hdr_dump)
{

    if (pthread_mutex_init(&hdr_dump->header_mutex, NULL) != 0)
    {
        return TRACEX_INIT_FAILURE;
    }

    return TRACEX_SUCCESS;
}

TRACEX_Ret_t tracex_header_get_header(struct tracex_header_dump_t *hdr_dump, struct TRACEX_header_t **header)
{
    /* Try to get the object mutex */
    pthread_mutex_lock(&hdr_dump->header_mutex);

    /* Check if the header has already been parsed */
    if (!hdr_dump->header_parsed)
    {
        /* Release the mutex before returning to the caller */
        pthread_mutex_unlock(&hdr_dump->header_mutex);
        return TRACEX_NEED_MORE;
    }

    /* Check if the header has been marked as invalid */
    if (hdr_dump->header_parsed && !hdr_dump->header_valid)
    {
        /* Release the mutex before returning to the caller */
        pthread_mutex_unlock(&hdr_dump->header_mutex);
        return TRACEX_HEADER_NOT_VALID;
    }

    /* Header is valid, return it to the user */
    *header = &hdr_dump->user_hdr;

    /* Release the mutex before returning to the caller */
    pthread_mutex_unlock(&hdr_dump->header_mutex);

    return TRACEX_SUCCESS;


}

TRACEX_Ret_t tracex_is_header_parsed(struct tracex_header_dump_t *hdr_dump)
{
    /* Try to get the object mutex */
    pthread_mutex_lock(&hdr_dump->header_mutex);

    if (!hdr_dump->header_parsed)
    {
        /* Release the mutex before returning to the caller */
        pthread_mutex_unlock(&hdr_dump->header_mutex);
        return TRACEX_NEED_MORE;
    }

    /* Release the mutex before returning to the caller */
    pthread_mutex_unlock(&hdr_dump->header_mutex);
    return TRACEX_SUCCESS;
}

TRACEX_Ret_t tracex_is_header_valid(struct tracex_header_dump_t *hdr_dump)
{
    /* Try to get the object mutex */
    pthread_mutex_lock(&hdr_dump->header_mutex);

    if (!hdr_dump->header_parsed)
    {
        /* Release the mutex before returning to the caller */
        pthread_mutex_unlock(&hdr_dump->header_mutex);
        return TRACEX_NEED_MORE;
    }

    if (!hdr_dump->header_valid)
    {
        /* Release the mutex before returning to the caller */
        pthread_mutex_unlock(&hdr_dump->header_mutex);
        return TRACEX_HEADER_NOT_VALID;
    }

    /* Release the mutex before returning to the caller */
    pthread_mutex_unlock(&hdr_dump->header_mutex);
    return TRACEX_SUCCESS;

}

TRACEX_Ret_t tracex_parse_header(struct tracex_header_dump_t *hdr_dump, void *buffer, size_t buff_len, uint64_t *consumed)
{
    TRACEX_Ret_t status;

    pthread_mutex_lock(&hdr_dump->header_mutex);
    
    *consumed = 0;
    status = parse_incrementally(hdr_dump, buffer, buff_len, consumed);

    if (status == TRACEX_SUCCESS)
    {
        /* We should have now parsed a full header */
        status = process_header(hdr_dump);

        if (status != TRACEX_SUCCESS)
        {
            hdr_dump->header_valid = 0;
        }
        else
        {
            /* Get the total number of objects in the object registry */
            hdr_dump->object_registry_size = tracex_object_compute_registry_size(
                hdr_dump->raw_hdr.obj_registry_start_pointer,
                hdr_dump->raw_hdr.obj_registry_end_pointer,
                hdr_dump->raw_hdr.obj_registry_name_size);
            
            /* Get the total number of events in the event trace buffer */
            hdr_dump->event_registry_size = tracex_event_compute_registry_size(
                hdr_dump->raw_hdr.buff_start_pointer,
                hdr_dump->raw_hdr.buff_end_pointer);

            hdr_dump->header_valid = 1;
        }

        /* Call the user provided callback */
        if (hdr_dump->parserCallback != NULL)
            hdr_dump->parserCallback(&hdr_dump->user_hdr, status);
    }
     
    goto handle_exit;
    
/* Release the mutex before returning to the caller */    
handle_exit:
    pthread_mutex_unlock(&hdr_dump->header_mutex);
    return status;

}

static TRACEX_Ret_t process_header(struct tracex_header_dump_t *hdr_dump)
{
    TRACEX_Ret_t status;
    struct tracex_raw_header_t *hdr;
    uint8_t *id;

    hdr = &hdr_dump->raw_hdr;

    id = (uint8_t*)&hdr->id;

    if (id[0] == 0x54 && id[1] == 0x58 && id[2] == 0x54 && id[3]== 0x42)
    {
        hdr_dump->user_hdr.endianess = E_TRACEX_BIG_ENDIAN;
    }
    else if (id[0] == 0x42 && id[1] == 0x54 && id[2] == 0x58 && id[3]== 0x54)
    {
        hdr_dump->user_hdr.endianess = E_TRACEX_LITTLE_ENDIAN;
    }
    else
    {
        status = TRACEX_HEADER_NOT_VALID;
        goto handle_exit;
    }

    if (hdr_dump->raw_hdr.obj_registry_name_size == 0)
    {
        status = TRACEX_OBJECT_REGISTRY_INVALID;
        goto handle_exit;
    }

    /* Check if the object registry is valid */
    if (tracex_object_check_registry_valid(hdr->obj_registry_start_pointer, hdr->obj_registry_end_pointer, hdr->obj_registry_name_size) != TRACEX_SUCCESS)
    {
        status = TRACEX_OBJECT_REGISTRY_INVALID;
        goto handle_exit;
    }

    if (tracex_event_check_registry_valid(hdr->buff_start_pointer, hdr->buff_end_pointer) != TRACEX_SUCCESS)
    {
        status = TRACEX_EVENT_TRACE_BUFFER_INVALID;
        goto handle_exit;
    }

    hdr_dump->user_hdr.object_name_size = hdr->obj_registry_name_size;
    hdr_dump->user_hdr.timer_mask = hdr->timer_valid_mask;

    status = TRACEX_SUCCESS;

handle_exit:
    return status;
}

static TRACEX_Ret_t parse_incrementally(struct tracex_header_dump_t *hdr_dump, void *buffer, size_t buff_len, uint64_t *consumed)
{
    TRACEX_Ret_t status;
    size_t bytes_to_copy;

    bytes_to_copy = 0;
    if (hdr_dump->byte_offset >= sizeof(struct tracex_raw_header_t))
    {
        status = TRACEX_HEADER_BAD_OFFSET_START;
        goto handle_exit;
    }

    if (buff_len + hdr_dump->byte_offset >= sizeof(struct tracex_raw_header_t))
    {
        bytes_to_copy = sizeof(struct tracex_raw_header_t) - hdr_dump->byte_offset;
    }
    else
    {
        bytes_to_copy = buff_len;

    }

    memcpy(((void*)&hdr_dump->raw_hdr)+ hdr_dump->byte_offset, buffer, bytes_to_copy);

    hdr_dump->byte_offset += bytes_to_copy;


    if (hdr_dump->byte_offset >= sizeof(struct tracex_raw_header_t))
    {
        hdr_dump->header_parsed = 1;
        status = TRACEX_SUCCESS;
    }

    else
    {
        status = TRACEX_NEED_MORE;
        goto handle_exit;
    }

handle_exit:
    *consumed = bytes_to_copy;
    return status;
}