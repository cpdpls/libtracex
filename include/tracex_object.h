#ifndef __TRACEX_OBJECT_H__
#define __TRACEX_OBJECT_H__

#include <stdint.h>
#include "tracex_errno.h"

typedef struct tracex_handler tracex_handler;   /* Forward declaration */

typedef struct tracex_obj_iterator TRACEX_object_iterator_t;

enum tracex_object_type
{
    TRACEX_OBJECT_TYPE_NOT_VALID            = 0,
    TRACEX_OBJECT_TYPE_THREAD               = 1,
    TRACEX_OBJECT_TYPE_TIMER                = 2,
    TRACEX_OBJECT_TYPE_QUEUE                = 3,
    TRACEX_OBJECT_TYPE_SEMAPHORE            = 4,
    TRACEX_OBJECT_TYPE_MUTEX                = 5,
    TRACEX_OBJECT_TYPE_EVENT_FLAGS_GROUP    = 6,
    TRACEX_OBJECT_TYPE_BLOCK_POOL           = 7,
    TRACEX_OBJECT_TYPE_BYTE_POOL            = 8,
    TRACEX_OBJECT_TYPE_MEDIA                = 9,
    TRACEX_OBJECT_TYPE_FILE                 = 10,
    TRACEX_OBJECT_TYPE_IP                   = 11,
    TRACEX_OBJECT_TYPE_PACKET_POOL          = 12,
    TRACEX_OBJECT_TYPE_TCP_SOCKET           = 13,
    TRACEX_OBJECT_TYPE_UDP_SOCKET           = 14,
    TRACEX_OBJECT_TYPE_RESERVED             = 15,
    TRACEX_OBJECT_TYPE_USB_HOST_STACK_DEV   = 21,
    TRACEX_OBJECT_TYPE_USB_HOST_STACK_INT   = 22,
    TRACEX_OBJECT_TYPE_USB_HOST_ENDPOINT    = 23,
    TRACEX_OBJECT_TYPE_USB_HOST_CLASS       = 24,
    TRACEX_OBJECT_TYPE_USB_DEV              = 25,
    TRACEX_OBJECT_TYPE_USB_DEV_INT          = 26,
    TRACEX_OBJECT_TYPE_USB_DEV_ENDPOINT     = 27,
    TRACEX_OBJECT_TYPE_USB_DEV_CLASS        = 28,
    TRACEX_OBJECT_TYPE_MAX                  = 29,
};

typedef struct
{
    uint32_t stack_start;
    uint32_t stack_size;

}tracex_thread_params_t;

typedef struct
{
    uint32_t initial_ticks;
    uint32_t reschedule_ticks;
}tracex_timer_params_t;

typedef struct
{
    uint32_t queue_size;
    uint32_t message_size;
}tracex_queue_params_t;

typedef struct
{
    uint32_t initial_instances;

}tracex_semaphore_params_t;

typedef struct
{
    uint32_t inheritance_flag;

} tracex_mutex_param_t;


typedef struct
{
    uint32_t total_blocks;
    uint32_t block_size;
} tracex_block_pool_params_t;

typedef struct
{
    uint32_t total_bytes;
} tracex_byte_pool_params_t;

typedef struct
{
    uint32_t fat_cache_size;
    uint32_t sector_cache_size;
}tracex_media_params_t;


typedef struct
{
    uint32_t stack_start;
    uint32_t stack_size;

} tracex_ip_params_t;

typedef struct
{
    uint32_t packet_size;
    uint32_t number_of_packets;
} tracex_packet_pool_t;

typedef struct
{
    uint32_t ip_addr;
    uint32_t window_size;
} tracex_tcp_socket_t;

typedef struct
{
    uint32_t ip_addr;
    uint32_t rx_queue_max;
} tracex_udp_socket_t;


struct tracex_object
{
    uint8_t  available;
    enum tracex_object_type type;
    union
    {
        uint16_t thread_priority;
        struct
        {
            uint8_t  res1;
            uint8_t  res2;
        };

    };

    uint32_t pointer;

    union
    {
        tracex_thread_params_t              thread;
        tracex_timer_params_t               timer;
        tracex_queue_params_t               queue;
        tracex_semaphore_params_t           semaphore;
        tracex_mutex_param_t                mutex;
        tracex_block_pool_params_t          blockPool;
        tracex_byte_pool_params_t           bytePool;
        tracex_media_params_t               media;
        tracex_ip_params_t                  ip;
        tracex_packet_pool_t                packetPool;
        tracex_tcp_socket_t                 tcpSocket;
        tracex_udp_socket_t                 udpSocket;

        /* Generic parameters */
        struct
        {
            uint32_t                        param_1;
            uint32_t                        param_2;
        };
    } objectParams;
    uint8_t     *name;

};



tracex_ret_t tracex_object_iterator_init(tracex_handler *handler, TRACEX_object_iterator_t **iterator);
tracex_ret_t tracex_object_iterator_next(TRACEX_object_iterator_t *iterator, const struct tracex_object **object);
void tracex_object_iterator_end(TRACEX_object_iterator_t **iterator);
const uint8_t *tracex_object_convert_type_to_string(enum tracex_object_type type);

#endif