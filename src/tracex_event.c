#include <stdlib.h>
#include <string.h>
#include "tracex_event_int.h"
#include "tracex_event.h"
#include "tracex_errno.h"
#include "tracex_core.h"
#include "tracex_event_str.h"

static void convert_from_raw_to_user(struct tracex_event_entry *entry);
static void destroy_event_entry(struct tracex_event_entry **entry);
static tracex_ret_t parse_incrementally(struct tracex_event_context *ctx, void *buffer, size_t buffer_len, size_t *consumed);
static tracex_ret_t process_event(struct tracex_event_context *ctx);
static tracex_ret_t alloc_new_event_entry(struct tracex_event_entry **entry_ptr);

tracex_ret_t tracex_event_int_init(struct tracex_event_context *ctx)
{

    /* Init the list of events */
    tracex_list_init(&ctx->event_list);

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
    event_buffer_entries = (stop - start) / sizeof(struct tracex_event_int);

    /* Check for an invalid registry size */
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
            /* Increment the number of events parsed for the current sessions*/
            ctx->curr_count++;

            status = process_event(ctx);
            
            if (status == TRACEX_SUCCESS)
            {
                ctx->tot_count++;
            }

            status = TRACEX_NEED_MORE;

            /* Check if we have parsed the whole event registry */
            if (ctx->curr_count == ctx->registry_size)
            {
                /* Reset the current session total event registry count */
                ctx->curr_count = 0;
                status = TRACEX_SUCCESS;
                goto handle_exit;
            }
        }

        /* Adjust the loop counter and the buffer position with it's size */
        buffer += bytes_consumed;
        bytes_left -= bytes_consumed;
    }

handle_exit:
    return status;

}


void tracex_event_int_destroy_list(struct tracex_event_context *ctx)
{
    struct tracex_event_entry *entry;
    struct tracex_event_entry *next;
    
    tracex_list_for_each_entry_safe(entry, next, &ctx->event_list, node)
    {
        destroy_event_entry(&entry);
    }
    
    /* In case we were parsing incrementally and that that the parsing got interrupted
     * We need to free the current working entry, otherwise this is a memory leak.
     * As it is not yet added to the current list.
     */
    if (ctx->curr_offset != 0 && ctx->curr_entry != NULL) {
        destroy_event_entry(&ctx->curr_entry);
    }

}

static tracex_ret_t parse_incrementally(struct tracex_event_context *ctx, void *buffer, size_t buffer_len, size_t *consumed)
{
    tracex_ret_t status;
    void *start_address;
    size_t bytes_to_copy;

    bytes_to_copy = 0;

    /* Check if this is a new event and zero out the structure */
    if (ctx->curr_offset == 0)
    {
        if ((status = alloc_new_event_entry(&ctx->curr_entry)) != TRACEX_SUCCESS)
        {
            goto handle_exit;
        }
    }

    /* Set the start address + the previous offset */
    start_address = (void*)&ctx->curr_entry->raw_event + ctx->curr_offset;

    /* Check if the buffer length added with the previous offset if bigger than a whole event struct */
    if (buffer_len + ctx->curr_offset >= sizeof(struct tracex_event_int))
    {
        /* Copy What is left to be copied */
        bytes_to_copy = sizeof(struct tracex_event_int) - ctx->curr_offset; 
    }

    /* Otherwise, there is not enough byte to parse a full event so copy what we can */
    else
    {
        bytes_to_copy = buffer_len;
    }

    ctx->curr_offset += bytes_to_copy;

    /* Perform the copy */
    memcpy(start_address, buffer, bytes_to_copy);

    /* Check if we reached the end of the parsing of a full event */
    if (ctx->curr_offset == sizeof(struct tracex_event_int))
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
    tracex_ret_t status;

    /*TODO: Revert to the correct endianess */

    /* Check if this is an empty invalid event */
    if (ctx->curr_entry->raw_event.thread_pointer == 0 || 
        ctx->curr_entry->raw_event.thread_priority == 0)
    {
        status = TRACEX_EVENT_INVALID;
        goto handle_exit;

    }

    /* Convert the raw objet to the user format */
    convert_from_raw_to_user(ctx->curr_entry);

    /* Add the event to the list */
    tracex_list_insert(&ctx->curr_entry->node, &ctx->event_list);
    
    status = TRACEX_SUCCESS;
    

handle_exit:
    if (status == TRACEX_SUCCESS) {
        /* Call the user provided callback */
        if (ctx->on_event_parsed != NULL)
            ctx->on_event_parsed(ctx->cb_data, &ctx->curr_entry->user_event, TRACEX_SUCCESS);
    } else {
        destroy_event_entry(&ctx->curr_entry);
    }
    return status;
}

static tracex_ret_t alloc_new_event_entry(struct tracex_event_entry **entry_ptr)
{
    tracex_ret_t status;
    struct tracex_event_entry *tmp_entry;

    /* Allocate a new event */
    tmp_entry = (struct tracex_event_entry*)malloc(sizeof(struct tracex_event_entry));
    if (tmp_entry == NULL)
    {
        status =  TRACEX_ALLOC_FAILURE;
        goto handle_exit;
    }

    /* Zero out the struct for safety */
    memset(tmp_entry, 0, sizeof(struct tracex_event_entry));
    status = TRACEX_SUCCESS;

handle_exit:
    *entry_ptr = tmp_entry;
    return status;

}

static void convert_from_raw_to_user(struct tracex_event_entry *entry)
{
    struct tracex_event_infos_labels info_labels;

    info_labels = tracex_event_infos_to_str(entry->raw_event.event_id);

    entry->user_event.eventId = entry->raw_event.event_id;
    entry->user_event.threadPointer = entry->raw_event.thread_pointer;
    entry->user_event.threadParams.threadPriority = entry->raw_event.thread_priority;
    entry->user_event.timeStamp = entry->raw_event.time_stamp;
    entry->user_event.rawInfos.info1 = entry->raw_event.info1;
    entry->user_event.rawInfos.info2 = entry->raw_event.info2;
    entry->user_event.rawInfos.info3 = entry->raw_event.info3;
    entry->user_event.rawInfos.info4 = entry->raw_event.info4;

    entry->user_event.info1Label = info_labels.info1_label;
    entry->user_event.info2Label = info_labels.info2_label;
    entry->user_event.info3Label = info_labels.info3_label;
    entry->user_event.info4Label = info_labels.info4_label;

   entry->user_event.eventLabel = tracex_event_id_to_str(entry->raw_event.event_id);

}
static void destroy_event_entry(struct tracex_event_entry **entry)
{
    if (entry != NULL)
    {
        if (*entry != NULL)
        {
            /* Check if the entry has been inserted in the list and delete it from it */
            if ((*entry)->node.next != NULL && (*entry)->node.prev != NULL)
            {
                tracex_list_delete(&(*entry)->node);
                (*entry)->node.next = NULL;
                (*entry)->node.prev = NULL;

            }

            free (*entry);
            *entry = NULL;
        }
    }
}