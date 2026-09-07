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

/*TODO: I misslead myself here. I should in the tracex_dump_t add a tracex_hdr_entry_t
*       Which will will contain the actual header_int with the valid flags and also the mutex
*       Same goes for the event and objects. There should be a proper object and event structure
*       That contains the flags and mutexes. This is so that the files tracex_event.c, 
*       tracex_header.c and tracex_obj.c only operates on their structures and do not depend
*       on tracex_core.h. Each module should opperate on it's data type to maintain code easier
*/
struct tracex_dump_t
{
    struct tracex_hdr_int_t     header;                 /* Header of the tracex dump*/
    struct tracex_list          obj_list;               /* List of objects */
    struct tracex_list          event_list;             /* List of events */
    uint64_t                    object_count;           /* Total count of objects */
    uint64_t                    event_count;            /* Total count of events */
};

struct TRACEX_user_dump_t
{
    struct TRACEX_header_t  header;         /* User tracex header */        
    struct TRACEX_object_t  **objects;      /* User list of objects */
    struct TRACEX_event_t   **events;       /* User list of events */
    uint64_t                object_count;   /* Total count of objects */
    uint64_t                event_count;    /* Total count of events */
};

struct TRACEX_handler_t
{
    struct tracex_dump_t        structured_raw;         /* Restructured dump from raw data */
    uint8_t                     header_parsed;          /* Flag set when the header has been parsed */
    uint8_t                     header_valid;           /* Flag set when the header is valid */
    size_t                      raw_bytes_count;        /* Total number of bytes of the dump */
    struct tracex_list          node;                   /* Pointers to the next TRACEX_handle_t node */
    enum tracex_state_t         state;                  /* Saved Finite State Machine between calls */
    uint16_t                    curr_obj_byte_count;    /* Saved current object byte count */
    uint16_t                    curr_event_byte_count;  /* Savec current event byte count */
    struct TRACEX_user_dump_t   user_dump;              /* User parsed dump */
    pthread_mutex_t             header_mutex;           /* Mutex used when retrieving and parsing objects */
    pthread_mutex_t             event_mutex;            /* Mutex used when retrieving and parsing events */
    pthread_mutex_t             object_mutex;           /* Mutex used when retrieving and parsing objects */
};

TRACEX_Ret_t tracex_is_handler_valid(struct TRACEX_handler_t *handler);

#endif