#ifndef __TRACEX_EVENT_H__
#define __TRACEX_EVENT_H__

#include <stdint.h>
#include "tracex_errno.h"



struct tracex_event
{
    uint32_t    thread_pointer;     /* Thread pointer when the event happened*/
    uint32_t    thread_priority;    /* Thread priority */
    uint32_t    event_id;           /* Event ID of the event */
    uint32_t    time_stamp;         /* Timestamp when the event happened*/
    uint32_t    info1;              /* Info 1 of the event */
    uint32_t    info2;              /* Info 2 of the event */
    uint32_t    info3;              /* Info 3 of the event */
    uint32_t    info4;              /* Info 4 of the event */
} __attribute__((__packed__));

#endif