#ifndef __TRACE_REGISTRY_H__
#define __TRACE_REGISTRY_H__

#include <stddef.h>
#include <stdint.h>

struct trace_object_entry
{
    uint8_t obj_available;
    uint8_t obj_type;
    uint8_t obj_res1;
    uint8_t obj_res2;
    uint32_t obj_ptr;
    uint32_t obj_param_1;
    uint32_t obj_param_2;
    uint8_t *obj_name;

}__attribute__((packed));

struct trace_parsed_registry
{
    struct trace_object_entry **objects;
    size_t object_size_in_buffer;
    size_t objects_count;

};

struct trace_registry
{
    struct trace_parsed_registry *parsed_registry;
    void *raw_buffer;
    size_t raw_buffer_size;
};
#endif

