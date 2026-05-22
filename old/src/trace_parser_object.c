#include "trace_ctrl.h"
#include "trace_parser_errno.h"
#include "trace_objects_registry.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "trace_parser_object.h"

static struct trace_objects_registry *alloc_registry(struct trace_control_header *header);
static struct trace_parsed_object_registry *alloc_parsed_registry(struct trace_objects_registry *obregistry, struct trace_control_header *header);
static struct trace_object_entry **alloc_objects(struct trace_parsed_object_registry *parsed_registry, struct trace_control_header *header);
static int read_registry_buffer(FILE *file_ptr, struct trace_control_header *header, struct trace_objects_registry *registry);
static int build_objects_index(struct trace_objects_registry *registry);
static void parsed_registry_destroy(struct trace_parsed_object_registry **parsed_registry);
static void objects_destroy(struct trace_object_entry ***objects);

char *object_type_strings[] = {
    "INVALID",
    "THREAD",
    "TIMER",
    "QUEUE",
    "SEMAPHORE",
    "MUTEX",
    "EVENT_FLAGS_GROUP",
    "BLOCK_POOL",
    "BYTE_POOL",
    "MEDIA",
    "FILE",
    "IP",
    "PACKET_POOL",
    "TCP",
    "TCP_SOCKET",
    "UDP_SOCKET",
    "RES1",
    "RES2",
    "RES3",
    "RES4",
    "RES5",
    "RES6",
    "USB_HOST_STACK_DEVICE",
    "USB_HOST_STACK_INTERFACE",
    "USB_HOST_ENDPOINT",
    "USB_HOST_CLASS",
    "USB_DEVICE",
    "USB_DEVICE_INTERFACE",
    "USB_DEVICE_ENDPOINT",
    "USB_DEVICE_CLASS",

};
int trace_parse_object_registry(FILE *file_ptr, struct trace_control_header *header, struct trace_objects_registry **object_registry)
{
    struct trace_objects_registry *tmp_registry;
    int status;

    if (object_registry == NULL)
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

    status = build_objects_index(tmp_registry);
    if( status != 0)
    {
        goto cleanup_registry_error;

    }
    
    *object_registry = tmp_registry;

    return 0;


cleanup_registry_error:
    trace_object_registry_destroy(&tmp_registry);

status_return:
    return status;
}
void trace_object_registry_destroy(struct trace_objects_registry **registry)
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

char  *trace_parse_object_get_type(unsigned int object_index, struct trace_objects_registry *object_registry)
{
    if (object_registry == NULL)
    {
        return "";
    }

    if (object_index > object_registry->parsed_registry->objects_count)
    {
        return "";
    }

    return object_type_strings[object_registry->parsed_registry->all_objects[object_index]->obj_type];
    
}
static int build_objects_index(struct trace_objects_registry *registry)
{
    size_t i;
    size_t in_use_count;
    
    /* Reset the real objects in use counter */
    in_use_count = 0;

    for (i = 0; i < registry->parsed_registry->objects_count; i++)
    {
        registry->parsed_registry->all_objects[i] = (struct trace_object_entry*)((registry->raw_buffer) + (i * registry->parsed_registry->object_size_in_buffer));

        /* Check if the object is truly a used one */
        if(registry->parsed_registry->all_objects[i]->obj_available  == 0)
        {   
            /* Build the used index */
            registry->parsed_registry->used_objects[in_use_count] = registry->parsed_registry->all_objects[i];
            in_use_count++;
        }
    }
    
    /* Strip down the used object index to the exact size */
    registry->parsed_registry->used_objects = realloc(registry->parsed_registry->used_objects, sizeof(struct trace_object_entry*) * in_use_count);
    if (registry->parsed_registry->used_objects == NULL)
    {
        return TRACE_PARSER_MEM_ERR;
    }
    else
    {
        /* Assign the counter to the internal structure */
        registry->parsed_registry->objects_in_use = in_use_count;
        return 0;
    }

}
static int read_registry_buffer(FILE *file_ptr, struct trace_control_header *header, struct trace_objects_registry *registry)
{
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
static struct trace_objects_registry *alloc_registry(struct trace_control_header *header)
{
    struct trace_objects_registry *tmp_registry;
    
    tmp_registry = (struct trace_objects_registry*)malloc(sizeof(struct trace_objects_registry));
    
    if (tmp_registry == NULL)
        return NULL;
    
    /* Compute the total size of the registry size */
    tmp_registry->raw_buffer_size = (   header->parsed_header->header_obj_registry_end_ptr - 
                                        header->parsed_header->header_obj_registry_start_ptr);
    
    /* Alloc the raw buffer */
    tmp_registry->raw_buffer = (void*)malloc(tmp_registry->raw_buffer_size);

    if (tmp_registry->raw_buffer == NULL)
        return NULL;
    
    /* Alloc the parsed registry structure */
    /* This is were the real size of an object is also computed */
    tmp_registry->parsed_registry = alloc_parsed_registry(tmp_registry, header);
    if (tmp_registry->parsed_registry == NULL)
    {
        return NULL;
    }
    return tmp_registry;

}
static struct trace_parsed_object_registry *alloc_parsed_registry(struct trace_objects_registry *registry, struct trace_control_header *header)
{
    struct trace_parsed_object_registry *tmp_parsed_registry;

    tmp_parsed_registry = (struct trace_parsed_object_registry*)malloc(sizeof(struct trace_parsed_object_registry));

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
    tmp_parsed_registry->all_objects = alloc_objects(tmp_parsed_registry, header);
    tmp_parsed_registry->used_objects = alloc_objects(tmp_parsed_registry, header);
    if (tmp_parsed_registry->all_objects == NULL || tmp_parsed_registry->used_objects == NULL)
    {
        return NULL;
    }
    return tmp_parsed_registry;

}
static struct trace_object_entry **alloc_objects(struct trace_parsed_object_registry *parsed_registry, struct trace_control_header *header)
{
    struct trace_object_entry **tmp_obj;

    tmp_obj = (struct trace_object_entry**)malloc(sizeof(struct trace_object_entry*) * parsed_registry->objects_count);
    
    if (tmp_obj == NULL)
        return NULL;
    
    return tmp_obj;

}
static void parsed_registry_destroy(struct trace_parsed_object_registry **parsed_registry)
{
    if (parsed_registry != NULL)
    {
        if (*parsed_registry != NULL)
        {
            objects_destroy(&(*parsed_registry)->all_objects);
            objects_destroy(&(*parsed_registry)->used_objects);
            free(*parsed_registry);
            *parsed_registry = NULL;
        }

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
