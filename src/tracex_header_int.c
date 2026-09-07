#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "tracex_core.h"
#include "tracex_errno.h"


TRACEX_Ret_t tracex_header_add_data(struct tracex_hdr_entry_t *hdr_entry, void *buffer, size_t buff_len, uint64_t *consumed)
{
    TRACEX_Ret_t status;
    size_t bytes_to_copy;

    
    if (hdr_entry == NULL || buffer == NULL)
    {
        status =  TRACEX_BAD_INPUT_PTR;
        goto handle_error;
    }

    if (hdr_entry->byte_offset >= sizeof(struct tracex_hdr_int_t))
    {
        status = TRACEX_HEADER_BAD_OFFSET_START;
        goto handle_error;
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

    goto handle_exit;

    /* We should have now have parsed a full header */
    
handle_error:
    hdr_entry->is_header_processed = 1;
    *consumed = 0;

handle_exit:
    return status;
}
TRACEX_Ret_t tracex_parse_data(struct tracex_hdr_entry_t *header)
{

}
TRACEX_Ret_t tracex_header_get_mask(struct tracex_hdr_entry_t *header, uint32_t *mask)
{
    if (header == NULL || mask == NULL)
    {
        return TRACEX_BAD_INPUT_PTR;
    }


    *mask = header->hdr.timer_valid_mask;
    return TRACEX_SUCCESS;
}