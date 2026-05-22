#ifndef __TRACE_REGISTRY_H__
#define __TRACE_REGISTRY_H__

#include <stddef.h>
#include <stdint.h>


enum object_type_t
{
    OBJ_INVALID,
    OBJ_THREAD,
    OBJ_TIMER,
    OBJ_QUEUE,
    OBJ_SEMAPHORE,
    OBJ_MUTEX,
    OBJ_EVENT_FLAGS_GROUP,
    OBJ_BLOCK_POOL,
    OBJ_BYTE_POOL,
    OBJ_MEDIA,
    OBJ_FILE,
    OBJ_IP,
    OBJ_PACKET_POOL,
    OBJ_TCP,
    OBJ_TCP_SOCKET,
    OBJ_UDP_SOCKET,
    OBJ_RES1,
    OBJ_RES2,
    OBJ_RES3,
    OBJ_RES4,
    OBJ_RES5,
    OBJ_RES6,
    OBJ_USB_HOST_STACK_DEVICE,
    OBJ_USB_HOST_STACK_INTERFACE,
    OBJ_USB_HOST_ENDPOINT,
    OBJ_USB_HOST_CLASS,
    OBJ_USB_DEVICE,
    OBJ_USB_DEVICE_INTERFACE,
    OBJ_USB_DEVICE_ENDPOINT,
    OBJ_USB_DEVICE_CLASS,
};

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

struct trace_parsed_object_registry
{
    struct trace_object_entry **all_objects;
    struct trace_object_entry **used_objects;
    size_t object_size_in_buffer;
    size_t objects_count;
    size_t objects_in_use;

};

struct trace_objects_registry
{
    struct trace_parsed_object_registry *parsed_registry;
    void *raw_buffer;
    size_t raw_buffer_size;
};
#endif

