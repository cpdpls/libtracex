#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "tracex_event_int.h"
#include "tracex_event.h"
#include "tracex_errno.h"
#include "tracex_core.h"


static tracex_ret_t parse_incrementally(struct tracex_event_context *ctx, void *buffer, size_t buffer_len, size_t *consumed);
static tracex_ret_t process_event(struct tracex_event_context *ctx);
static tracex_ret_t alloc_new_event_array_block(struct tracex_event_array_block **block, uint64_t elements);
static void destroy_event_array_block(struct tracex_event_array_block **block);
static void destroy_event_array_block_list(struct tracex_list *list);

tracex_ret_t tracex_event_int_init(struct tracex_event_context *ctx)
{
    if (pthread_mutex_init(&ctx->mutex, NULL) != 0)
    {
        return TRACEX_INIT_FAILURE;
    }

    /* Init the different used lists */
    tracex_list_init(&ctx->event_list);
    tracex_list_init(&ctx->array_block_list);

    return TRACEX_SUCCESS;
}

tracex_ret_t tracex_event_int_compute_registry_size(uint64_t *registry_size, uint32_t start, uint32_t stop)
{
    tracex_ret_t status;
    uint64_t event_buffer_entries;

    event_buffer_entries = 0;

    /*TODO: Maybe find other checks in here */
    if (stop == start)
    {
        status = TRACEX_EVENT_TRACE_BUFFER_INVALID;
        goto handle_exit;
    }

    /* Compute the total possible event entries inside the buffer */
    event_buffer_entries = (stop - start) / sizeof(struct tracex_event);

    if (event_buffer_entries == 0)
    {
        status = TRACEX_EVENT_TRACE_BUFFER_INVALID;
        goto handle_exit;
    }

    status = TRACEX_SUCCESS;

handle_exit:
    *registry_size = event_buffer_entries;
    return status;
}

tracex_ret_t tracex_event_int_parse(struct tracex_event_context *ctx, void *buffer, size_t buff_len, uint64_t *consumed)
{
    tracex_ret_t status;
    size_t bytes_left;
    size_t bytes_consumed;

    pthread_mutex_lock(&ctx->mutex);

    *consumed = 0;
    bytes_left = buff_len;

    while (bytes_left != 0)
    {
        bytes_consumed = 0;

        status = parse_incrementally(ctx, buffer, bytes_left, &bytes_consumed);
        /* Sanitize for an error */
        if (status != TRACEX_SUCCESS && status != TRACEX_NEED_MORE)
        {
            goto handle_exit;
        }

        /* Increment the number of bytes consumed for the caller */
        *consumed += bytes_consumed;

        /* We parsed a full event, let's now process it and add it to the list */
        if (status == TRACEX_SUCCESS)
        {
            status = process_event(ctx);
            
            if (status == TRACEX_SUCCESS)
            {
                /* Increment the true valid counter and the total one*/
                ctx->cycle_valid_count++;
                ctx->total_events++;
                
                /* Update the current entry pointer inside the current block of events array */
                ctx->curr_entry = &ctx->current_block->entries[ctx->cycle_valid_count];
            }

            /* Increment the number of events parsed for the current sessions even if not valid */
            ctx->cycle_count++;

            status = TRACEX_NEED_MORE;

            /* Check if we have parsed the whole event registry */
            if (ctx->cycle_count == ctx->registry_size)
            {
                /* Shrink the entries to the true parsed events count */
                ctx->current_block->entries = (struct tracex_event_entry *)realloc(ctx->current_block->entries, sizeof(struct tracex_event_entry) * ctx->cycle_valid_count);
                if (ctx->current_block->entries == NULL)
                {
                    /* TODO: If we fail, we might need to decrement the number of total events parsed */
                    /* Additionally, we need to remote the total of bytes processed. */
                    /* Maybe also change the buffer to be **buffer, so that we can decrement that and give */
                    /* The user the positibility to try again ... */

                    ctx->total_events -= ctx->cycle_valid_count;
                    status = TRACEX_ALLOC_FAILURE;
                    goto handle_exit;
                }
                /* Reset the current session total event registry count */
                ctx->cycle_count = 0;
                ctx->cycle_valid_count = 0;

                status = TRACEX_SUCCESS;
                goto handle_exit;
            }
        }

        /* Adjust the loop counter and the buffer position with it's size */
        buffer += bytes_consumed;
        bytes_left -= bytes_consumed;
    }

handle_exit:
    pthread_mutex_unlock(&ctx->mutex);
    return status;

}


void tracex_event_int_destroy_list(struct tracex_event_context *ctx)
{
    struct tracex_event_entry *entry_event;
    struct tracex_event_entry *next_event;
    
    tracex_list_for_each_entry_safe(entry_event, next_event, &ctx->event_list, node)
    {
        tracex_list_delete(&entry_event->node);
        entry_event->node.next = NULL;
        entry_event->node.prev = NULL;
    }

    destroy_event_array_block_list(&ctx->array_block_list);

}

static tracex_ret_t parse_incrementally(struct tracex_event_context *ctx, void *buffer, size_t buffer_len, size_t *consumed)
{
    tracex_ret_t status;
    void *start_address;
    size_t bytes_to_copy;

    bytes_to_copy = 0;

    /* Check if we are at the beginning of a parsing cycle */
    if (ctx->cycle_count == 0)
    {
        /* Allocate a new block that contains an array of elements  */
        if ((status = alloc_new_event_array_block(&ctx->current_block, ctx->registry_size)) != TRACEX_SUCCESS)
        {
            goto handle_exit;
        }

        /* Add the newly created block of events array inside the list */
        tracex_list_insert(&ctx->current_block->node ,&ctx->array_block_list);

        /* Set the current entry to point to the start of the newly created block */
        ctx->curr_entry = &ctx->current_block->entries[0];
    }

    /* Set the start address + the previous offset */
    start_address = (void*)&ctx->curr_entry->event + ctx->curr_offset;

    /* Check if the buffer length added with the previous offset if bigger than a whole event struct */
    if (buffer_len + ctx->curr_offset >= sizeof(struct tracex_event))
    {
        /* Copy What is left to be copied */
        bytes_to_copy = sizeof(struct tracex_event) - ctx->curr_offset; 
    }

    /* Otherwise, there is not enough byte to parse a full event so copy what we can */
    else
    {
        bytes_to_copy = buffer_len;
        ctx->curr_offset += bytes_to_copy;
    }

    /* Perform the copy */
    memcpy(start_address, buffer, bytes_to_copy);

    /* Check if we reached the end of the parsing of a full event */
    if (ctx->curr_offset + bytes_to_copy == sizeof(struct tracex_event))
    {
        /* Reset the current offset */
        ctx->curr_offset = 0;
        status = TRACEX_SUCCESS;
    }
    else
    {
        status = TRACEX_NEED_MORE;
    }

handle_exit:
    *consumed = bytes_to_copy;
    return status;

}

static tracex_ret_t process_event(struct tracex_event_context *ctx)
{
    /*TODO: Revert to the correct endianess */


    /* Add the event to the list */
    tracex_list_insert(&ctx->curr_entry->node, &ctx->event_list);

    /* Call the user provided callback */
    if (ctx->user_callback != NULL)
        ctx->user_callback(&ctx->curr_entry->event, TRACEX_SUCCESS);

    return TRACEX_SUCCESS;
}

static tracex_ret_t alloc_new_event_array_block(struct tracex_event_array_block **block, uint64_t elements)
{
    tracex_ret_t status;
    struct tracex_event_array_block *tmp_block;

    /* Allocate a new block of block */
    tmp_block = (struct tracex_event_array_block*)malloc(sizeof(struct tracex_event_array_block));
    if (tmp_block == NULL)
    {
        status = TRACEX_ALLOC_FAILURE;
        goto handle_exit;
    }

    /* Zero out the block elements for safety */
    memset(tmp_block, 0, sizeof(struct tracex_event_array_block));

    /* Allocate the actual array of entries */
    tmp_block->entries = (struct tracex_event_entry*)malloc(sizeof(struct tracex_event_entry) * elements);
    if (tmp_block->entries == NULL)
    {
        status = TRACEX_ALLOC_FAILURE;
        goto handle_error;
    }

    /* Zero out the array of events for safety */
    memset(tmp_block->entries, 0, sizeof(struct tracex_event_entry) * elements);

    status = TRACEX_SUCCESS;

    goto handle_exit;


handle_error:
    destroy_event_array_block(&tmp_block);

handle_exit:
    *block = tmp_block;
    return status;

}

static void destroy_event_array_block(struct tracex_event_array_block **block)
{
    if (block != NULL)
    {
        if (*block != NULL)
        {
            free((*block)->entries);
            (*block)->entries = NULL;
            tracex_list_delete(&(*block)->node);
            (*block)->node.next = NULL;
            (*block)->node.prev = NULL;

            free((*block));
            (*block) = NULL;
        }
    }
}

static void destroy_event_array_block_list(struct tracex_list *list)
{
    struct tracex_event_array_block *entry;
    struct tracex_event_array_block *next;

    tracex_list_for_each_entry_safe(entry, next, list, node)
    {
        destroy_event_array_block(&entry);
    }
}