#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <pthread.h>

#include "tracex_core.h"
#include "tracex_errno.h"

TRACEX_Ret_t process_header(struct tracex_header_dump_t *hdr_dump);

TRACEX_Ret_t tracex_init_header(struct tracex_header_dump_t *hdr_dump)
{

    if (pthread_mutex_init(&hdr_dump->header_mutex, NULL) != 0)
    {
        return TRACEX_INIT_FAILURE;
    }

    return TRACEX_SUCCESS;
}

TRACEX_Ret_t tracex_header_get_header(struct tracex_header_dump_t *dump, struct TRACEX_header_t **header)
{
    /* Try to get the object mutex */
    pthread_mutex_lock(&dump->header_mutex);

    /* Check if the header has already been parsed */
    if (!dump->header_parsed)
    {
        /* Release the mutex before returning to the caller */
        pthread_mutex_unlock(&dump->header_mutex);
        return TRACEX_NEED_MORE;
    }

    /* Check if the header has been marked as invalid */
    if (dump->header_parsed && !dump->header_valid)
    {
        /* Release the mutex before returning to the caller */
        pthread_mutex_unlock(&dump->header_mutex);
        return TRACEX_HEADER_NOT_VALID;
    }

    /* Header is valid, return it to the user */
    *header = &dump->user_hdr;

    /* Release the mutex before returning to the caller */
    pthread_mutex_unlock(&dump->header_mutex);

    return TRACEX_SUCCESS;


}


TRACEX_Ret_t tracex_parse_header(struct tracex_header_dump_t *dump, void *buffer, size_t buff_len, uint64_t *consumed)
{
    TRACEX_Ret_t status;
    size_t bytes_to_copy;

    pthread_mutex_lock(&dump->header_mutex);

    if (dump->byte_offset >= sizeof(struct tracex_raw_header_t))
    {
        status = TRACEX_HEADER_BAD_OFFSET_START;
        goto handle_exit;
    }

    if (buff_len + dump->byte_offset >= sizeof(struct tracex_raw_header_t))
    {
        bytes_to_copy = sizeof(struct tracex_raw_header_t) - dump->byte_offset;
    }
    else
    {
        bytes_to_copy = buff_len;

    }

    memcpy(((void*)&dump->raw_hdr)+ dump->byte_offset, buffer, bytes_to_copy);

    *consumed = bytes_to_copy;
    dump->byte_offset += bytes_to_copy;


    if (dump->byte_offset >= sizeof(struct tracex_raw_header_t))
    {
        dump->header_parsed = 1;
        status = TRACEX_SUCCESS;
    }

    else
    {
        status = TRACEX_NEED_MORE;
        goto handle_exit;
    }
    
    /* We should have now have parsed a full header */

    status = process_header(dump);

    if (status != TRACEX_SUCCESS)
    {
        dump->header_valid = 0;
    }
    else
    {
        dump->header_valid = 1;
    }
    
    goto handle_exit;
    
/* Release the mutex before returning to the caller */    
handle_exit:
    pthread_mutex_unlock(&dump->header_mutex);
    return status;

}

TRACEX_Ret_t process_header(struct tracex_header_dump_t *hdr_dump)
{
    TRACEX_Ret_t status;
    uint8_t *id;

    id = (uint8_t*)&hdr_dump->raw_hdr.id;

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
        status = TRACEX_HEADER_NOT_VALID;
        goto handle_exit;
    }

    if (hdr_dump->raw_hdr.obj_registry_end_pointer == hdr_dump->raw_hdr.obj_registry_start_pointer)
    {
        status = TRACEX_HEADER_NOT_VALID;
        goto handle_exit;
    }

    if (hdr_dump->raw_hdr.buff_end_pointer == hdr_dump->raw_hdr.buff_start_pointer)
    {
        status = TRACEX_HEADER_NOT_VALID;
        goto handle_exit;
    }

    hdr_dump->user_hdr.object_name_size = hdr_dump->raw_hdr.obj_registry_name_size;
    hdr_dump->user_hdr.timer_mask = hdr_dump->raw_hdr.timer_valid_mask;

    status = TRACEX_SUCCESS;

handle_exit:
    return status;
}