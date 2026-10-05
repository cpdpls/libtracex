#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "tracex_core.h"
#include "tracex/tracex_errno.h"
#include "tracex_obj_int.h"

static tracex_ret_t process_header(struct tracex_header_context *ctx);
static tracex_ret_t parse_incrementally(struct tracex_header_context *ctx, void *buffer, size_t buff_len, uint64_t *consumed);
static tracex_ret_t alloc_new_user_header(struct tracex_header **user_hdr);
static void destroy_user_header(struct tracex_header **user_hdr);
static void convert_header_from_raw_to_user(struct tracex_header_raw *raw, struct tracex_header *user);

void tracex_header_destroy_context(struct tracex_header_context *ctx)
{
	destroy_user_header(&ctx->user_header);
}

tracex_ret_t tracex_header_int_check_parsed(struct tracex_header_context *ctx)
{
    if (!ctx->header_parsed)
    {
        
        return TRACEX_NEED_MORE;
    }

    return TRACEX_SUCCESS;
}

tracex_ret_t tracex_header_int_check_valid(struct tracex_header_context *ctx)
{
    
    if (!ctx->header_parsed)
    {   
        return TRACEX_NEED_MORE;
    }

    if (!ctx->header_valid)
    {
        
        return TRACEX_HEADER_NOT_VALID;
    }

    
    return TRACEX_SUCCESS;

}

tracex_ret_t tracex_header_int_get(struct tracex_header_context *ctx, struct tracex_header **header)
{
    

    /* Check if the header has already been parsed */
    if (!ctx->header_parsed)
    {
       
        return TRACEX_NEED_MORE;
    }

    /* Check if the header has been marked as invalid */
    if (ctx->header_parsed && !ctx->header_valid)
    {
        
        return TRACEX_HEADER_NOT_VALID;
    }

    /* Header is valid, return it to the user */
    *header = ctx->user_header;

    return TRACEX_SUCCESS;


}

tracex_ret_t tracex_header_int_parse(struct tracex_header_context *ctx, void *buffer, size_t buff_len, uint64_t *consumed)
{
    tracex_ret_t status;

    status = parse_incrementally(ctx, buffer, buff_len, consumed);

    if (status == TRACEX_SUCCESS)
    {
        /* We should have now parsed a full header */
        status = process_header(ctx);

	    convert_header_from_raw_to_user(&ctx->staging_raw_header, ctx->user_header);

	if (status != TRACEX_SUCCESS)
            ctx->header_valid = 0;
        else
            ctx->header_valid = 1;

        /* Call the user provided callback */
        if (ctx->on_header_parsed != NULL)
            ctx->on_header_parsed(ctx->cb_data, ctx->user_header, status);
    }
    
    return status;

}

static tracex_ret_t process_header(struct tracex_header_context *ctx)
{
    tracex_ret_t status;
    struct tracex_header_raw *hdr = NULL;
    uint8_t *id = NULL;

    hdr = &ctx->staging_raw_header;

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
    if (tracex_object_int_compute_registry_size(
            &ctx->obj_registry_size,
            hdr->obj_registry_start_ptr,
            hdr->obj_registry_end_ptr,
            hdr->obj_registry_name_size) != TRACEX_SUCCESS)
    {
        status = TRACEX_OBJECT_REGISTRY_INVALID;
        goto handle_exit;
    }

    if (tracex_event_int_compute_registry_size(
            &ctx->event_registry_size,
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
    size_t bytes_to_copy = 0;

    /* Check if we are beginning to parser the header. Zero-out the staging header and allocate a user header */
    if (ctx->staging_raw_offset == 0)
    {
	    memset(&ctx->staging_raw_header, 0, sizeof(struct tracex_header_raw));

        if ((status = alloc_new_user_header(&ctx->user_header)) != TRACEX_SUCCESS)
		    goto handle_exit;
    }

    if (ctx->staging_raw_offset >= sizeof(struct tracex_header_raw))
    {
        status = TRACEX_HEADER_BAD_OFFSET_START;
        goto handle_exit;
    }

    if (buff_len + ctx->staging_raw_offset >= sizeof(struct tracex_header_raw))
    {
        bytes_to_copy = sizeof(struct tracex_header_raw) - ctx->staging_raw_offset;
    }
    else
    {
        bytes_to_copy = buff_len;

    }

    memcpy((void*)((size_t)&ctx->staging_raw_header + (size_t)ctx->staging_raw_offset), buffer, bytes_to_copy);

    ctx->staging_raw_offset += bytes_to_copy;


    if (ctx->staging_raw_offset >= sizeof(struct tracex_header_raw))
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

static tracex_ret_t alloc_new_user_header(struct tracex_header **user_hdr)
{
	tracex_ret_t status;
	struct tracex_header *tmp_hdr = NULL;

    /* Allocate a new user header */

	tmp_hdr = (struct tracex_header *)malloc(sizeof(struct tracex_header));
    if (tmp_hdr == NULL) {
	    status = TRACEX_ALLOC_FAILURE;
	    goto handle_exit;
    }

    /* Zero-out the struct for safety */
    memset(tmp_hdr, 0, sizeof(struct tracex_header));
    status = TRACEX_SUCCESS;

handle_exit:
	*user_hdr = tmp_hdr;
	return status;
}

static void destroy_user_header(struct tracex_header **user_hdr)
{
    if (user_hdr != NULL) {
        if (*user_hdr != NULL) {
		    free(*user_hdr);
		    *user_hdr = NULL;
	    }
    }
}
static void convert_header_from_raw_to_user(struct tracex_header_raw *raw, struct tracex_header *user)
{
	user->Id = raw->id;
	user->obj_registry_name_size = raw->obj_registry_name_size;
	user->timeStampMask = raw->timestamp_mask;
}