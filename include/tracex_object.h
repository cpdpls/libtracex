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
    /* 15-20 reserved */
    TRACEX_OBJECT_TYPE_USB_HOST_STACK_DEV   = 21,
    TRACEX_OBJECT_TYPE_USB_HOST_STACK_INT   = 22,
    TRACEX_OBJECT_TYPE_USB_HOST_ENDPOINT    = 23,
    TRACEX_OBJECT_TYPE_USB_HOST_CLASS       = 24,
    TRACEX_OBJECT_TYPE_USB_DEV              = 25,
    TRACEX_OBJECT_TYPE_USB_DEV_INT          = 26,
    TRACEX_OBJECT_TYPE_USB_DEV_ENDPOINT     = 27,
    TRACEX_OBJECT_TYPE_USB_DEV_CLASS        = 28,
    TRACEX_OBJECT_TYPE_MAX                  = 30,
};

struct tracex_object
{
    enum tracex_object_type type;
    const uint8_t *objectTypeLabel;
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
        struct { uint32_t stack_start;      uint32_t stack_size;        } thread;
        struct { uint32_t initial_ticks;    uint32_t reschedule_ticks;  } timer;
        struct { uint32_t queue_size;       uint32_t message_size;      } queue;
        struct { uint32_t initial_count;    uint32_t unused;            } semaphore;
        struct { uint32_t inheritance_flag; uint32_t unused;            } mutex;
        struct { uint32_t total_blocks;     uint32_t block_size;        } block_pool;
        struct { uint32_t total_bytes;      uint32_t unused;            } byte_pool;
        struct { uint32_t fat_cache_size;   uint32_t sector_cache_size; } media;
        struct { uint32_t stack_start;      uint32_t stack_size;        } ip;
        struct { uint32_t packet_size;      uint32_t num_packets;       } packet_pool;
        struct { uint32_t ip_address;       uint32_t window_size;       } tcp_socket;
        struct { uint32_t ip_address;       uint32_t rx_queue_max;      } udp_socket;

        /* Default fallback */
        struct { uint32_t param1; uint32_t param2; } raw;
    } params;

    const uint8_t *param1Label;
    const uint8_t *param2Label;
    uint8_t     *name;

};



tracex_ret_t tracex_object_iterator_init(tracex_handler *handler, TRACEX_object_iterator_t **iterator);
tracex_ret_t tracex_object_iterator_next(TRACEX_object_iterator_t *iterator, const struct tracex_object **object);
void tracex_object_iterator_end(TRACEX_object_iterator_t **iterator);

#endif