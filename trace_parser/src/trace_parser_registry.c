#include "trace_ctrl.h"
#include "trace_parser_errno.h"
#include "trace_registry.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "trace_parser_registry.h"

static struct trace_registry *alloc_registry(struct trace_control_header *header);
static struct trace_parsed_registry *alloc_parsed_registry(struct trace_registry *registry, struct trace_control_header *header);
static struct trace_object_entry **alloc_objects(struct trace_parsed_registry *parsed_registry, struct trace_control_header *header);
static int read_registry_buffer(FILE *file_ptr, struct trace_control_header *header, struct trace_registry *registry);
static void assign_objects_ptrs(struct trace_registry *registry);
static void parsed_registry_destroy(struct trace_parsed_registry **parsed_registry);
static void objects_destroy(struct trace_object_entry ***objects);

int trace_parse_registry(FILE *file_ptr, struct trace_control_header *header, struct trace_registry **registry)
{
    struct trace_registry *tmp_registry;
    int status;

    if (registry == NULL)
    {
        status = TRACE_PARSER_INVALID_PTR;
        goto status_return;
    }
    
    /* Allocate memory for the registry */
    tmp_registry = alloc_registry(header);

    if (tmp_registry == NULL)
    {
        status = TRACE_PARSER_MEM_ERR;
        goto cleanup_registry_error;
    }
    
    /* Read the registry inside the raw buffer */
    status = read_registry_buffer(file_ptr, header, tmp_registry);

    if (status != 0)
    {
        goto cleanup_registry_error;
    }

    assign_objects_ptrs(tmp_registry);
    
    *registry = tmp_registry;

    return 0;


cleanup_registry_error:
    trace_registry_destroy(&tmp_registry);

status_return:
    return status;
}
void trace_registry_destroy(struct trace_registry **registry)
{
    uint64_t i;

    if (registry != NULL)
    {
        if (*registry != NULL)
        {
            if ((*registry)->raw_buffer != NULL)
            {
                free((*registry)->raw_buffer);
                (*registry)->raw_buffer = NULL;
            }

            parsed_registry_destroy(&(*registry)->parsed_registry);

            free(*registry);

            *registry = NULL;
        }
    }
}

static void assign_objects_ptrs(struct trace_registry *registry)
{
    size_t i;

    for (i = 0; i < registry->parsed_registry->objects_count; i++)
    {
        registry->parsed_registry->objects[i] = (struct trace_object_entry*)((registry->raw_buffer) + (i * registry->parsed_registry->object_size_in_buffer));
    }

}
static int read_registry_buffer(FILE *file_ptr, struct trace_control_header *header, struct trace_registry *registry)
{
    size_t bytes_read;
    size_t offset;
    size_t read_bytes;
    
    offset = header->parsed_header->header_obj_registry_start_ptr - header->parsed_header->header_trace_base_addr;
    /* Place the read pointer at the exact position in the file descriptor */
    if (fseek(file_ptr, offset, SEEK_SET) != 0)
    {
        return TRACE_PARSER_FILE_OP_ERROR;
    }

    read_bytes = fread(registry->raw_buffer,
                         1,
                         registry->raw_buffer_size,
                         file_ptr);

    if (read_bytes != registry->raw_buffer_size ||
        (registry->parsed_registry->object_size_in_buffer * registry->parsed_registry->objects_count) != registry->raw_buffer_size)
    {
        return TRACE_PARSER_CORRUPTED_OBJECTS;
    }

    return 0;


}
static struct trace_registry *alloc_registry(struct trace_control_header *header)
{
    struct trace_registry *tmp_registry;
    
    tmp_registry = (struct trace_registry*)malloc(sizeof(struct trace_registry));
    
    if (tmp_registry == NULL)
        return NULL;
    
    /* Compute the total size of the registry size */
    tmp_registry->raw_buffer_size = (   header->parsed_header->header_obj_registry_end_ptr - 
                                        header->parsed_header->header_obj_registry_start_ptr);
    
    /* Alloc the raw buffer */
    tmp_registry->raw_buffer = (void*)malloc(tmp_registry->raw_buffer_size);

    if (tmp_registry->raw_buffer == NULL)
    {
        return NULL;
    }
    
    /* Alloc the parsed registry structure */
    /* This is were the real size of an object is also computed */
    tmp_registry->parsed_registry = alloc_parsed_registry(tmp_registry, header);
    if (tmp_registry->parsed_registry == NULL)
    {
        return NULL;
    }
    return tmp_registry;

}
static struct trace_parsed_registry *alloc_parsed_registry(struct trace_registry *registry, struct trace_control_header *header)
{
    struct trace_parsed_registry *tmp_parsed_registry;

    tmp_parsed_registry = (struct trace_parsed_registry*)malloc(sizeof(struct trace_parsed_registry));

    if (tmp_parsed_registry == NULL)
    {
        return NULL;
    }
    
    /* Compute the real object size inside the buffer */
    tmp_parsed_registry->object_size_in_buffer = (sizeof(struct trace_object_entry) - 
                                                sizeof(uint8_t*) +
                                                header->parsed_header->header_obj_registry_name_size);
    
    /* We can now compute the number of real object inside the buffer */
    tmp_parsed_registry->objects_count = registry->raw_buffer_size/tmp_parsed_registry->object_size_in_buffer;
    
    /* Allocate the pointers to the objects inside the buffer */
    tmp_parsed_registry->objects = alloc_objects(tmp_parsed_registry, header);
    if (tmp_parsed_registry == NULL)
    {
        return NULL;
    }
    return tmp_parsed_registry;

}
static struct trace_object_entry **alloc_objects(struct trace_parsed_registry *parsed_registry, struct trace_control_header *header)
{
    struct trace_object_entry **tmp_obj;

    tmp_obj = (struct trace_object_entry**)malloc(sizeof(struct trace_object_entry*) * parsed_registry->objects_count);
    
    if (tmp_obj == NULL)
        return NULL;
    
    return tmp_obj;

}
static void parsed_registry_destroy(struct trace_parsed_registry **parsed_registry)
{
    if (parsed_registry != NULL)
    {
        if (*parsed_registry != NULL)
        {
            objects_destroy(&(*parsed_registry)->objects);
        }

        free(*parsed_registry);
        *parsed_registry = NULL;
    }

}
static void objects_destroy(struct trace_object_entry ***objects)
{
    if (objects != NULL)
    {
        if (*objects != NULL)
            free(*objects);

        *objects = NULL;
    }

}
