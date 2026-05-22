#include "trace_parser_event.h"
#include "trace_ctrl.h"
#include "trace_events.h"
#include "trace_objects_registry.h"
#include "trace_parser_errno.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static struct trace_events_registry *alloc_event_registry(struct trace_control_header *header);
static struct trace_event_entry **alloc_event_entries(struct trace_parsed_event_registry *parsed_registry, struct trace_control_header *header);
static struct trace_parsed_event_registry *alloc_parsed_event_registry(struct trace_events_registry *registry, struct trace_control_header *header);
static int read_event_registry_buffer(FILE *file_ptr, struct trace_control_header *header, struct trace_events_registry *event_registry);
static void build_event_index(struct trace_events_registry *registry);
static void parsed_registry_destroy(struct trace_parsed_event_registry **parsed_registry);
static void events_destroy(struct trace_event_entry ***entries);

int trace_parse_event_registry(FILE *file_ptr, struct trace_control_header *header, struct trace_events_registry **event_registry)
{
    struct trace_events_registry *tmp_events;
    int status;
    
    tmp_events = NULL;

    if (event_registry == NULL)
    {
        status = TRACE_PARSER_INVALID_PTR;
        goto status_return;
    }

    /* Allocate memory for the registry */

    tmp_events = alloc_event_registry(header);

    if (tmp_events == NULL)
    {
        status = TRACE_PARSER_MEM_ERR;
        goto status_return;
    }

    status = read_event_registry_buffer(file_ptr, header, tmp_events);

    if (status != 0)
    {
        goto cleanup_registry_error;
    }

    build_event_index(tmp_events);
    
    *event_registry = tmp_events;
    return 0;

cleanup_registry_error:
    trace_event_registry_destroy(&tmp_events);

status_return:
    return status;

}
void trace_event_registry_destroy(struct trace_events_registry **event_registry)
{
    if (event_registry != NULL)
    {
        if (*event_registry != NULL)
        {
            if ((*event_registry)->raw_buffer != NULL)
            {
                free((*event_registry)->raw_buffer);
                (*event_registry)->raw_buffer = NULL;
            }

            parsed_registry_destroy(&(*event_registry)->parsed_registry);
            free(*event_registry);
            *event_registry = NULL;

        }
    }

}
static void build_event_index(struct trace_events_registry *registry)
{
    size_t i;

    for (i = 0; i < registry->parsed_registry->events_count; i++)
    {
        registry->parsed_registry->events[i] = (struct trace_event_entry*)((registry->raw_buffer) + (i * sizeof(struct trace_event_entry)));
    }

}

static struct trace_events_registry *alloc_event_registry(struct trace_control_header *header)
{
    struct trace_events_registry *tmp_registry;

    tmp_registry = (struct trace_events_registry*)malloc(sizeof(struct trace_events_registry));

    if (tmp_registry == NULL)
        return NULL;

    /* Compute the total size of the registry size */
    tmp_registry->raw_buffer_size = (header->parsed_header->header_buffer_end_ptr -
                                     header->parsed_header->header_buffer_start_ptr);

    /* Alloc the raw buffer */
    tmp_registry->raw_buffer = (void*)malloc(tmp_registry->raw_buffer_size);

    if (tmp_registry->raw_buffer == NULL)
        return NULL;
    
    /* Alloc the parsed registry structure */
    
    tmp_registry->parsed_registry = alloc_parsed_event_registry(tmp_registry, header);

    if (tmp_registry->parsed_registry == NULL)
        return NULL;

    tmp_registry->parsed_registry->events = alloc_event_entries(tmp_registry->parsed_registry, header);

    if (tmp_registry->parsed_registry->events == NULL)
        return NULL;


    return tmp_registry;

}
static struct trace_event_entry **alloc_event_entries(struct trace_parsed_event_registry *parsed_registry, struct trace_control_header *header)
{
    struct trace_event_entry **tmp_obj;

    tmp_obj = (struct trace_event_entry**)malloc(sizeof(struct trace_event_entry*) * parsed_registry->events_count);
    if (tmp_obj == NULL)
        return NULL;

    return tmp_obj;
}
static struct trace_parsed_event_registry *alloc_parsed_event_registry(struct trace_events_registry *registry, struct trace_control_header *header)
{
    struct trace_parsed_event_registry *tmp_event_registry;

    tmp_event_registry = (struct trace_parsed_event_registry*)malloc(sizeof(struct trace_parsed_event_registry));

    if (tmp_event_registry == NULL)
        return NULL;

    tmp_event_registry->events_count = registry->raw_buffer_size / sizeof(struct trace_event_entry);

    return tmp_event_registry;
}
static int read_event_registry_buffer(FILE *file_ptr, struct trace_control_header *header, struct trace_events_registry *event_registry)
{
    size_t bytes_read;
    size_t offset;

    offset = header->parsed_header->header_buffer_start_ptr - header->parsed_header->header_trace_base_addr;

    if (fseek(file_ptr, offset, SEEK_SET) != 0)
    {
        return TRACE_PARSER_FILE_OP_ERROR;
    }

    if (ftell(file_ptr) != offset)
    {
        return TRACE_PARSER_FILE_OP_ERROR;
    }

    bytes_read = fread(event_registry->raw_buffer,
                       1,
                       event_registry->raw_buffer_size,
                       file_ptr);

    if (bytes_read != event_registry->raw_buffer_size || (event_registry->parsed_registry->events_count * sizeof(struct trace_event_entry) != bytes_read))
    {
        return TRACE_PARSER_CORRUPTED_EVENTS;
    }

    return 0;
}
static void parsed_registry_destroy(struct trace_parsed_event_registry **parsed_registry)
{
    if (parsed_registry != NULL)
    {
        if (*parsed_registry != NULL)
        {
            events_destroy(&(*parsed_registry)->events);
            free(*parsed_registry);
            *parsed_registry = NULL;
        }
    }
}
static void events_destroy(struct trace_event_entry ***entries)
{
    if (entries != NULL)
    {
        if (*entries != NULL)
            free(*entries);

        *entries = NULL;
    }

}
