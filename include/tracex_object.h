#ifndef __TRACEX_OBJECT_H__
#define __TRACEX_OBJECT_H__

#include <stdint.h>
#include "tracex_errno.h"

typedef struct tracex_handler tracex_handler;   /* Forward declaration */

// enum TRACEX_ObjectType
// {
//     TRACEX_OBJECT_TYPE_NOT_VALID            = 0,
//     TRACEX_OBJECT_TYPE_THREAD               = 1,
//     TRACEX_OBJECT_TYPE_TIMER                = 2,
//     TRACEX_OBJECT_TYPE_QUEUE                = 3,
//     TRACEX_OBJECT_TYPE_SEMAPHORE            = 4,
//     TRACEX_OBJECT_TYPE_MUTEX                = 5,
//     TRACEX_OBJECT_TYPE_EVENT_FLAGS_GROUP    = 6,
//     TRACEX_OBJECT_TYPE_BLOCK_POOL           = 7,
//     TRACEX_OBJECT_TYPE_BYTE_POOL            = 8,
//     TRACEX_OBJECT_TYPE_MEDIA                = 9,
//     TRACEX_OBJECT_TYPE_FILE                 = 10,
//     TRACEX_OBJECT_TYPE_IP                   = 11,
//     TRACEX_OBJECT_TYPE_PACKET_POOL          = 12,
//     TRACEX_OBJECT_TYPE_TCP_SOCKET           = 13,
//     TRACEX_OBJECT_TYPE_UDP_SOCKET           = 14,
//     TRACEX_OBJECT_TYPE_RESERVED             = 15,
//     TRACEX_OBJECT_TYPE_USB_HOST_STACK_DEV   = 21,
//     TRACEX_OBJECT_TYPE_USB_HOST_STACK_INT   = 22,
//     TRACEX_OBJECT_TYPE_USB_HOST_ENDPOINT    = 23,
//     TRACEX_OBJECT_TYPE_USB_HOST_CLASS       = 24,
//     TRACEX_OBJECT_TYPE_USB_DEV              = 25,
//     TRACEX_OBJECT_TYPE_USB_DEV_INT          = 26,
//     TRACEX_OBJECT_TYPE_USB_DEV_ENDPOINT     = 27,
//     TRACEX_OBJECT_TYPE_USB_DEV_CLASS        = 28,
// };

// typedef struct
// {
//     uint32_t stack_start;
//     uint32_t stack_size;

// }TRACEX_thread_params_t;

// typedef struct
// {
//     uint32_t initial_ticks;
//     uint32_t reschedule_ticks;
// }TRACEX_timer_params_t;

// typedef struct
// {
//     uint32_t queue_size;
//     uint32_t message_size;
// }TRACEX_queue_params_t;

// typedef struct
// {
//     uint32_t initial_instances;

// }TRACEX_semaphore_params_t;

// typedef struct
// {
//     uint32_t inheritance_flag;

// } TRACEX_mutex_param_t;


// typedef struct
// {
//     uint32_t total_blocks;
//     uint32_t block_size;
// } TRACEX_block_pool_params_t;

// typedef struct
// {
//     uint32_t total_bytes;
// } TRACEX_byte_pool_params_t;

// typedef struct
// {
//     uint32_t fat_cache_size;
//     uint32_t sector_cache_size;
// }TRACEX_media_params_t;


// typedef struct
// {
//     uint32_t stack_start;
//     uint32_t stack_size;

// } TRACEX_ip_params_t;

// typedef struct
// {
//     uint32_t packet_size;
//     uint32_t number_of_packets;
// } TRACEX_packet_pool_t;

// typedef struct
// {
//     uint32_t ip_addr;
//     uint32_t window_size;
// } TRACEX_tcp_socket_t;

// typedef struct
// {
//     uint32_t ip_addr;
//     uint32_t rx_queue_max;
// } TRACEX_udp_socket_t;
typedef struct tracex_obj_iterator TRACEX_object_iterator_t;

struct tracex_object
{
    uint8_t  available;
    uint8_t  type;
    uint8_t  res1;
    uint8_t  res2;
    uint32_t pointer;
    uint32_t param_1;
    uint32_t param_2;
    uint8_t  *name;

}__attribute__((__packed__));



tracex_ret_t tracex_object_iterator_init(tracex_handler *handler, TRACEX_object_iterator_t **iterator);
tracex_ret_t tracex_object_iterator_next(TRACEX_object_iterator_t *iterator, const struct tracex_object **object);
void tracex_object_iterator_end(TRACEX_object_iterator_t **iterator);
#endif