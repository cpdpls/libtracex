#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "tracex_event_int.h"
#include "tracex_event.h"
#include "tracex_errno.h"
#include "tracex_core.h"


static TRACEX_Ret_t parse_incrementally(struct tracex_event_dump_t *dump, void *buffer, size_t buffer_len, size_t *consumed);
static TRACEX_Ret_t process_event(struct tracex_event_dump_t *dump);
static TRACEX_Ret_t alloc_new_event_entry(struct tracex_event_entry_t **event);

uint64_t tracex_event_compute_registry_size(uint32_t start, uint32_t stop)
{
    return (stop - start) / sizeof(struct tracex_event_raw_t);
}

TRACEX_Ret_t tracex_init_events(struct tracex_event_dump_t *event_dump, struct tracex_header_dump_t *hdr_dump)
{
    if (pthread_mutex_init(&event_dump->event_mutex, NULL) != 0)
    {
        return TRACEX_INIT_FAILURE;
    }
    event_dump->hdr_dump_ptr = hdr_dump;

    tracex_list_init(&event_dump->event_list);

    return TRACEX_SUCCESS;
}

TRACEX_Ret_t tracex_event_check_registry_valid(uint32_t start, uint32_t stop)
{
    /*TODO: Maybe find other checks in here */
    if (stop == start)
    {
        return TRACEX_EVENT_TRACE_BUFFER_INVALID;
    }

    return TRACEX_SUCCESS;
}

TRACEX_Ret_t tracex_parse_events(struct tracex_event_dump_t *event_dump, void *buffer, size_t buff_len, uint64_t *consumed)
{
    TRACEX_Ret_t status;
    size_t bytes_left;
    size_t bytes_consumed;

    pthread_mutex_lock(&event_dump->event_mutex);

    *consumed = 0;
    bytes_left = buff_len;

    while (bytes_left != 0)
    {
        bytes_consumed = 0;

        status = parse_incrementally(event_dump, buffer, bytes_left, &bytes_consumed);
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
            event_dump->curr_event_count++;

            status = process_event(event_dump);
            
            if (status == TRACEX_ALLOC_FAILURE)
            {
                goto handle_exit;
            }
            
            if (status == TRACEX_SUCCESS)
            {
                event_dump->tot_event_count++;
            }

            status = TRACEX_NEED_MORE;

            /* Check if we have parsed the whole event registry */
            if (event_dump->curr_event_count == event_dump->hdr_dump_ptr->event_registry_size)
            {
                /* Reset the current session total event registry count */
                event_dump->curr_event_count = 0;
                status = TRACEX_SUCCESS;
                goto handle_exit;
            }
        }

        /* Adjust the loop counter and the buffer position with it's size */
        buffer += bytes_consumed;
        bytes_left -= bytes_consumed;
    }

handle_exit:
    pthread_mutex_unlock(&event_dump->event_mutex);
    return status;

}

void tracex_destroy_event(struct tracex_event_entry_t **entry)
{
    if (entry != NULL)
    {
        if (*entry != NULL)
        {
            tracex_list_delete(&(*entry)->node);
            (*entry)->node.next = NULL;
            (*entry)->node.prev = NULL;

            free (*entry);
            *entry = NULL;
        }
    }
}

void tracex_destroy_event_list(struct tracex_event_dump_t *event_dump)
{
    struct tracex_event_entry_t *entry;
    struct tracex_event_entry_t *next;
    
    if (&event_dump->event_list != NULL)
    {
        tracex_list_for_each_entry_safe(entry, next, &event_dump->event_list, node)
        {
            tracex_destroy_event(&entry);
        }

    }
}

static TRACEX_Ret_t parse_incrementally(struct tracex_event_dump_t *dump, void *buffer, size_t buffer_len, size_t *consumed)
{
    TRACEX_Ret_t status;
    void *start_address;
    size_t bytes_to_copy;

    bytes_to_copy = 0;

    /* Check if this is a new event and zero out the structure */
    if (dump->curr_event_offset == 0)
    {
        /* Zero out the structure */
        memset(&dump->current_raw_event, 0, sizeof(struct tracex_event_raw_t));
    }

    /* Set the start address + the previous offset */
    start_address = (void*)&dump->current_raw_event + dump->curr_event_offset;

    /* Check if the buffer length added with the privious offset if bigger than a whole event struct */
    if (buffer_len + dump->curr_event_offset >= sizeof(struct tracex_event_raw_t))
    {
        /* Copy What is left to be copied */
        bytes_to_copy = sizeof(struct tracex_event_raw_t) - dump->curr_event_offset; 
    }

    /* Otherwise, there is not enough byte to parse a full event so copy what we can */
    else
    {
        bytes_to_copy = buffer_len;
        dump->curr_event_offset += bytes_to_copy;
    }

    /* Perform the copy */
    memcpy(start_address, buffer, bytes_to_copy);

    /* Check if we reached the end of the parsing of a full event */
    if (dump->curr_event_offset + bytes_to_copy == sizeof(struct tracex_event_raw_t))
    {
        /* Reset the current offset */
        dump->curr_event_offset = 0;
        status = TRACEX_SUCCESS;
    }
    else
    {
        status = TRACEX_NEED_MORE;
    }

    *consumed = bytes_to_copy;
    return status;

}

static TRACEX_Ret_t process_event(struct tracex_event_dump_t *dump)
{
    TRACEX_Ret_t status;
    struct tracex_event_entry_t *user_event;

    /*TODO: If we fail, we need to decrement the number of parsed bytes */
    if (alloc_new_event_entry(&user_event) != TRACEX_SUCCESS)
    {
        status = TRACEX_ALLOC_FAILURE;
        goto handle_exit;
    }

    /*TODO: Assign the correct endianess */
    user_event->event.thread_pointer = dump->current_raw_event.tx_trace_buffer_entry_thread_pointer;
    user_event->event.thread_priority = dump->current_raw_event.tx_trace_buffer_entry_thread_priority;
    user_event->event.event_id = dump->current_raw_event.tx_trace_buffer_entry_event_id;
    user_event->event.time_stamp = dump->current_raw_event.tx_trace_buffer_entry_time_stamp;
    user_event->event.info1 = dump->current_raw_event.tx_trace_buffer_entry_information_field_1;
    user_event->event.info2 = dump->current_raw_event.tx_trace_buffer_entry_information_field_2;
    user_event->event.info3 = dump->current_raw_event.tx_trace_buffer_entry_information_field_3;
    user_event->event.info4 = dump->current_raw_event.tx_trace_buffer_entry_information_field_4;

    /* Add the event to the list */
    tracex_list_insert(&user_event->node, &dump->event_list);

    
    status = TRACEX_SUCCESS;
    
handle_exit:
    /* Call the user provided callback */
    if (dump->parserCallback != NULL)
        dump->parserCallback(&user_event->event, status);

    return status;
}

static TRACEX_Ret_t alloc_new_event_entry(struct tracex_event_entry_t **event)
{
    struct tracex_event_entry_t *tmp_event;

    /* Allocate a new event */
    tmp_event = (struct tracex_event_entry_t*)malloc(sizeof(struct tracex_event_entry_t));
    if (tmp_event == NULL)
    {
        return TRACEX_ALLOC_FAILURE;
    }

    /* Zero out the struct for safety */
    memset(tmp_event, 0, sizeof(struct tracex_event_entry_t));
    *event = tmp_event;
    return TRACEX_SUCCESS;


}