#ifndef __TRACEX_OBJECT_H__
#define __TRACEX_OBJECT_H__

#include <stdint.h>
#include "tracex_errno.h"

typedef struct tracex_handler tracex_handler;   /* Forward declaration */

typedef struct tracex_obj_iterator TRACEX_object_iterator_t;

struct tracex_object
{
    uint32_t type;
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

    uint8_t     *name;

    void *user_data;

    void (*user_data_destructor)(void *user_data);
};

#endif