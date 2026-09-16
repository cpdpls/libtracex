#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <pthread.h>

#include "tracex_core.h"
#include "tracex_errno.h"
#include "tracex_obj_int.h"

static tracex_ret_t process_header(struct tracex_header_context *ctx);
static tracex_ret_t parse_incrementally(struct tracex_header_context *ctx, void *buffer, size_t buff_len, uint64_t *consumed);

tracex_ret_t tracex_header_init(struct tracex_header_context *ctx)
{

    if (pthread_mutex_init(&ctx->header_mutex, NULL) != 0)
    {
        return TRACEX_INIT_FAILURE;
    }

    return TRACEX_SUCCESS;
}

tracex_ret_t tracex_header_check_parsed(struct tracex_header_context *ctx)
{
    /* Try to get the object mutex */
    pthread_mutex_lock(&ctx->header_mutex);

    if (!ctx->header_parsed)
    {
        /* Release the mutex before returning to the caller */
        pthread_mutex_unlock(&ctx->header_mutex);
        return TRACEX_NEED_MORE;
    }

    /* Release the mutex before returning to the caller */
    pthread_mutex_unlock(&ctx->header_mutex);
    return TRACEX_SUCCESS;
}

tracex_ret_t tracex_header_check_valid(struct tracex_header_context *ctx)
{
    /* Try to get the object mutex */
    pthread_mutex_lock(&ctx->header_mutex);

    if (!ctx->header_parsed)
    {
        /* Release the mutex before returning to the caller */
        pthread_mutex_unlock(&ctx->header_mutex);
        return TRACEX_NEED_MORE;
    }

    if (!ctx->header_valid)
    {
        /* Release the mutex before returning to the caller */
        pthread_mutex_unlock(&ctx->header_mutex);
        return TRACEX_HEADER_NOT_VALID;
    }

    /* Release the mutex before returning to the caller */
    pthread_mutex_unlock(&ctx->header_mutex);
    return TRACEX_SUCCESS;

}

tracex_ret_t tracex_header_get(struct tracex_header_context *ctx, struct tracex_header **header)
{
    /* Try to get the object mutex */
    pthread_mutex_lock(&ctx->header_mutex);

    /* Check if the header has already been parsed */
    if (!ctx->header_parsed)
    {
        /* Release the mutex before returning to the caller */
        pthread_mutex_unlock(&ctx->header_mutex);
        return TRACEX_NEED_MORE;
    }

    /* Check if the header has been marked as invalid */
    if (ctx->header_parsed && !ctx->header_valid)
    {
        /* Release the mutex before returning to the caller */
        pthread_mutex_unlock(&ctx->header_mutex);
        return TRACEX_HEADER_NOT_VALID;
    }

    /* Header is valid, return it to the user */
    *header = &ctx->header;

    /* Release the mutex before returning to the caller */
    pthread_mutex_unlock(&ctx->header_mutex);

    return TRACEX_SUCCESS;


}

tracex_ret_t tracex_header_parse(struct tracex_header_context *ctx, void *buffer, size_t buff_len, uint64_t *consumed)
{
    tracex_ret_t status;

    pthread_mutex_lock(&ctx->header_mutex);
    
    *consumed = 0;
    status = parse_incrementally(ctx, buffer, buff_len, consumed);

    if (status == TRACEX_SUCCESS)
    {
        /* We should have now parsed a full header */
        status = process_header(ctx);

        if (status != TRACEX_SUCCESS)
            ctx->header_valid = 0;
        else
            ctx->header_valid = 1;

        /* Call the user provided callback */
        if (ctx->user_callback != NULL)
            ctx->user_callback(&ctx->header, status);
    }
     
    goto handle_exit;
    
/* Release the mutex before returning to the caller */    
handle_exit:
    pthread_mutex_unlock(&ctx->header_mutex);
    return status;

}

static tracex_ret_t process_header(struct tracex_header_context *ctx)
{
    tracex_ret_t status;
    struct tracex_header *hdr;
    uint8_t *id;

    hdr = &ctx->header;

    id = (uint8_t*)&hdr->id;

    if (id[0] == 0x54 && id[1] == 0x58 && id[2] == 0x54 && id[3]== 0x42)
    {
        ctx->endianess = E_TRACEX_BIG_ENDIAN;
    }
    else if (id[0] == 0x42 && id[1] == 0x54 && id[2] == 0x58 && id[3]== 0x54)
    {
        ctx->endianess = E_TRACEX_LITTLE_ENDIAN;
    }
    else
    {
        status = TRACEX_HEADER_NOT_VALID;
        goto handle_exit;
    }

    if (hdr->obj_registry_name_size == 0)
    {
        status = TRACEX_OBJECT_REGISTRY_INVALID;
        goto handle_exit;
    }

    /* Check if the object registry is valid */
    if (tracex_object_compute_registry_size(
            ctx->obj_entry,
            hdr->obj_registry_start_ptr,
            hdr->obj_registry_end_ptr,
            hdr->obj_registry_name_size) != TRACEX_SUCCESS)
    {
        status = TRACEX_OBJECT_REGISTRY_INVALID;
        goto handle_exit;
    }

    if (tracex_event_compute_registry_size(
            ctx->event_entry,
            hdr->event_buff_start_ptr,
            hdr->event_buff_end_ptr) != TRACEX_SUCCESS)
    {
        status = TRACEX_EVENT_TRACE_BUFFER_INVALID;
        goto handle_exit;
    }

    status = TRACEX_SUCCESS;

handle_exit:
    return status;
}

static tracex_ret_t parse_incrementally(struct tracex_header_context *ctx, void *buffer, size_t buff_len, uint64_t *consumed)
{
    tracex_ret_t status;
    size_t bytes_to_copy;

    bytes_to_copy = 0;
    if (ctx->byte_offset >= sizeof(struct tracex_header))
    {
        status = TRACEX_HEADER_BAD_OFFSET_START;
        goto handle_exit;
    }

    if (buff_len + ctx->byte_offset >= sizeof(struct tracex_header))
    {
        bytes_to_copy = sizeof(struct tracex_header) - ctx->byte_offset;
    }
    else
    {
        bytes_to_copy = buff_len;

    }

    memcpy(((void*)&ctx->header)+ ctx->byte_offset, buffer, bytes_to_copy);

    ctx->byte_offset += bytes_to_copy;


    if (ctx->byte_offset >= sizeof(struct tracex_header))
    {
        ctx->header_parsed = 1;
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