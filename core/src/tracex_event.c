#include <stdlib.h>
#include <string.h>
#include "tracex/tracex_event.h"
#include "tracex/tracex_errno.h"
#include "tracex_event_int.h"
#include "tracex_core.h"

/* In case a resolver engine is not registed or, the engine returns an error,
 * This string will be assigned to all the event info strings 
 */

static char *default_resolver_invalid_info_str = "Not valid";

/* In case a resolver engine is not registed or, the engine returns an error,
 * This string will be assigned to the event name
 */
static char *default_resolver_invalid_event_str = "Invalid";


static void convert_from_raw_to_user(struct tracex_event_context *ctx, struct tracex_event_raw *raw, struct tracex_event *user);
static void resolve_labels(struct tracex_event_context *ctx, struct tracex_event *event);
static void destroy_event_entry(struct tracex_event_entry **entry);
static tracex_ret_t parse_incrementally(struct tracex_event_context *ctx, void *buffer, size_t buffer_len, size_t *consumed);
static tracex_ret_t process_event(struct tracex_event_context *ctx);
static tracex_ret_t alloc_new_event_entry(struct tracex_event_entry **entry_ptr);
static void destroy_events_list(struct tracex_event_context *ctx);

tracex_ret_t tracex_event_int_init(struct tracex_event_context *ctx)
{

    /* Init the list of events */
    tracex_list_init(&ctx->event_list);

    return TRACEX_SUCCESS;
}

tracex_ret_t tracex_event_int_compute_registry_size(uint64_t *registry_size, uint32_t start, uint32_t stop)
{
    tracex_ret_t status;
    uint64_t event_buffer_entries = 0;

    /*TODO: Maybe find other checks in here */
    if (stop == start)
    {
        status = TRACEX_EVENT_TRACE_BUFFER_INVALID;
        goto handle_exit;
    }

    /* Compute the total possible event entries inside the buffer */
    event_buffer_entries = (stop - start) / sizeof(struct tracex_event_raw);

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

tracex_ret_t tracex_event_int_register_resolver_function(struct tracex_event_context *ctx, tracexResolverGetlabel resolverFunc)
{
	tracex_ret_t status;

    /* Sanitize input */
    if (ctx == NULL || resolverFunc == NULL) {
	    status = TRACEX_BAD_INPUT_PTR;
	    goto handle_exit;
    }

    /* Assign the function resolver to the context */
    ctx->resolverFunc = resolverFunc;
    
    status = TRACEX_SUCCESS;

handle_exit:
	return status;
}

tracex_ret_t tracex_event_int_refresh_resolver_labels(struct tracex_event_context *ctx)
{
    tracex_ret_t status;
    struct tracex_event_entry *entry;

    /* Sanitize input */
    if (ctx == NULL) {
	    status = TRACEX_BAD_INPUT_PTR;
	    goto handle_exit;
    }

    /* Since the resolve function has parsed or deleted labels
     * We need to loop on the already parsed object and update their labels 
     */

     tracex_list_for_each_entry(entry, &ctx->event_list, node) {
	    resolve_labels(ctx, &entry->event);
    }

    status = TRACEX_SUCCESS;

handle_exit:
	return status;
}
tracex_ret_t tracex_event_int_parse(struct tracex_event_context *ctx, void *buffer, size_t buff_len, uint64_t *consumed)
{
    tracex_ret_t status;
    size_t bytes_left = buff_len;
    size_t bytes_consumed = 0;

    *consumed = 0;
    status = TRACEX_NEED_MORE;

    while (bytes_left != 0) {
	    bytes_consumed = 0;

	    status = parse_incrementally(ctx, buffer, bytes_left, &bytes_consumed);
	    /* Sanitize for an error */
	    if (status != TRACEX_SUCCESS && status != TRACEX_NEED_MORE) {
		    goto handle_exit;
	    }

	    /* Increment the number of bytes consumed for the caller */
	    *consumed += bytes_consumed;

	    /* We parsed a full event, let's now process it and add it to the list */
	    if (status == TRACEX_SUCCESS) {
		    /* Increment the number of events parsed for the current sessions*/
		    ctx->curr_count++;

		    status = process_event(ctx);

		    if (status == TRACEX_SUCCESS) {
			    ctx->tot_count++;
		    }

		    status = TRACEX_NEED_MORE;

		    /* Check if we have parsed the whole event registry */
		    if (ctx->curr_count == ctx->registry_size) {
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


void tracex_event_int_destroy_context(struct tracex_event_context *ctx)
{
    /* Destroy the list of parsed events */
	destroy_events_list(ctx);

    /* In case we were parsing incrementally and that that the parsing got interrupted
     * We need to free the current working entry, otherwise this is a memory leak.
     * As it is not yet added to the current list.
     */
    if (ctx->staging_raw_offset != 0 && ctx->tmp_event != NULL) {
        destroy_event_entry(&ctx->tmp_event);
    }

}

static tracex_ret_t parse_incrementally(struct tracex_event_context *ctx, void *buffer, size_t buffer_len, size_t *consumed)
{
    tracex_ret_t status;
    void *start_address = NULL;
    size_t bytes_to_copy = 0;


    /* Check if this is a new event. Zero out the staging structure and allocate a new temp event */
    if (ctx->staging_raw_offset == 0)
    {
	    memset(&ctx->staging_raw_event, 0, sizeof(struct tracex_event_raw));
	    if ((status = alloc_new_event_entry(&ctx->tmp_event)) != TRACEX_SUCCESS)
	    {
	        goto handle_exit;
	    }
    }

    /* Set the start address + the previous offset */
    start_address = (void*)((size_t)&ctx->staging_raw_event + (size_t)ctx->staging_raw_offset);

    /* Check if the buffer length added with the previous offset if bigger than a whole event struct */
    if (buffer_len + ctx->staging_raw_offset >= sizeof(struct tracex_event_raw))
    {
        /* Copy What is left to be copied */
        bytes_to_copy = sizeof(struct tracex_event_raw) - ctx->staging_raw_offset; 
    }

    /* Otherwise, there is not enough byte to parse a full event so copy what we can */
    else
    {
        bytes_to_copy = buffer_len;
    }

    ctx->staging_raw_offset += bytes_to_copy;

    /* Perform the copy */
    memcpy(start_address, buffer, bytes_to_copy);

    /* Check if we reached the end of the parsing of a full event */
    if (ctx->staging_raw_offset == sizeof(struct tracex_event_raw))
    {
        /* Reset the current offset */
        ctx->staging_raw_offset = 0;
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
    if (ctx->staging_raw_event.thread_pointer == 0 || 
        ctx->staging_raw_event.thread_priority == 0)
    {
        status = TRACEX_EVENT_INVALID;
        goto handle_exit;

    }

    /* Convert the raw objet to the user format */
    convert_from_raw_to_user(ctx, &ctx->staging_raw_event, &ctx->tmp_event->event);

    /* Add the event to the list */
    tracex_list_insert(&ctx->tmp_event->node, &ctx->event_list);
    
    status = TRACEX_SUCCESS;
    

handle_exit:
    if (status == TRACEX_SUCCESS) {
        /* Call the user provided callback */
        if (ctx->on_event_parsed != NULL)
            ctx->on_event_parsed(ctx->cb_data, &ctx->tmp_event->event, TRACEX_SUCCESS);
    } else {
        destroy_event_entry(&ctx->tmp_event);
    }
    return status;
}

static tracex_ret_t alloc_new_event_entry(struct tracex_event_entry **entry_ptr)
{
    tracex_ret_t status;
    struct tracex_event_entry *tmp_entry = NULL;

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

static void convert_from_raw_to_user(struct tracex_event_context *ctx, struct tracex_event_raw *raw, struct tracex_event *user)
{
	tracex_resolver_labels labels;

	user->eventId = raw->event_id;
	user->threadPointer = raw->thread_pointer;
	user->threadPriority = raw->thread_priority;
	user->eventId = raw->event_id;
	user->timeStamp = raw->time_stamp;

	user->rawInfos.info1 = raw->info1;
	user->rawInfos.info2 = raw->info2;
	user->rawInfos.info3 = raw->info3;
	user->rawInfos.info4 = raw->info4;

	resolve_labels(ctx, user);
}

static void resolve_labels(struct tracex_event_context *ctx, struct tracex_event *event)
{
    tracex_resolver_labels labels;

     if (ctx->resolverFunc != NULL) {
	    labels = ctx->resolverFunc(E_TRACEX_RESOLVER_EVENT, event->eventId);

	    event->labels.event_name = (labels.eventLabels.event_name == NULL) ? default_resolver_invalid_event_str :
								  labels.eventLabels.event_name;
	    event->labels.info1 = (labels.eventLabels.info1 == NULL) ? default_resolver_invalid_info_str :
                                    labels.eventLabels.info1;


        event->labels.info2 = (labels.eventLabels.info2 == NULL) ? default_resolver_invalid_info_str :
                                    labels.eventLabels.info2;
        event->labels.info3 = (labels.eventLabels.info3 == NULL) ? default_resolver_invalid_info_str :
                                    labels.eventLabels.info3;

        event->labels.info4 = (labels.eventLabels.info4 == NULL) ? default_resolver_invalid_info_str :
                                    labels.eventLabels.info4;


    } else {
	    event->labels.event_name = default_resolver_invalid_event_str;
	    event->labels.info1 = default_resolver_invalid_info_str;
        event->labels.info2 = default_resolver_invalid_info_str;
        event->labels.info3 = default_resolver_invalid_info_str;
        event->labels.info4 = default_resolver_invalid_info_str;
    }


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

static void destroy_events_list(struct tracex_event_context *ctx)
{
    struct tracex_event_entry *entry = NULL;
    struct tracex_event_entry *next = NULL;
    
    tracex_list_for_each_entry_safe(entry, next, &ctx->event_list, node)
    {
	    destroy_event_entry(&entry);
    }

}