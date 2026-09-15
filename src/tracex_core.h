#ifndef __TRACEX_CORE_H__
#define __TRACEX_CORE_H__

#include <stdint.h>
#include <pthread.h>
#include "tracex_errno.h"
#include "tracex_header_int.h"
#include "tracex_obj_int.h"
#include "tracex_event_int.h"
#include "tracex_list.h"
#include "tracex_header.h"
#include "tracex_event.h"
#include "tracex_object.h"

enum tracex_state_t
{
    E_HEADER_PHASE,                                     /* Header phase FSM */
    E_OBJECT_PHASE,                                     /* Object phase FSM */
    E_EVENT_PHASE,                                      /* Event phase FSM */
};


struct tracex_raw_dump_t
{
    struct tracex_header_dump_t     header;                 /* Header of the tracex dump*/
    struct tracex_object_dump_t     objs;                   /* Objects of the tracex dump */
    struct tracex_event_dump_t      events;                 /* Events of the tracex dump */
};

struct TRACEX_user_dump_t
{
    struct TRACEX_header_t  header;         /* User tracex header */        
    struct TRACEX_event_t   **events;       /* User list of events */
    uint64_t                event_count;    /* Total count of events */
};

struct TRACEX_handler_t
{
    struct tracex_raw_dump_t    raw_dump;               /* Restructured dump from raw data */
    size_t                      raw_bytes_count;        /* Total number of bytes of the dump */
    struct tracex_list          node;                   /* Pointers to the next TRACEX_handle_t node */
    enum tracex_state_t         state;                  /* Saved Finite State Machine between calls */
    // uint64_t                    curr_event_byte_count;  /* Savec current event byte count */
    // pthread_mutex_t             event_mutex;            /* Mutex used when retrieving and parsing events */
    // pthread_mutex_t             object_mutex;           /* Mutex used when retrieving and parsing objects */
};


TRACEX_Ret_t tracex_is_handler_valid(struct TRACEX_handler_t *handler);

#endif