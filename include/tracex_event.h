#ifndef __TRACEX_EVENT_H__
#define __TRACEX_EVENT_H__

#include <stdint.h>
#include "tracex_errno.h"


typedef struct TRACEX_handler_t TRACEX_handler_t;       /* Forward declaration */

enum TRACEX_event_id_t
{
    E_TEST_A,

};

enum TRACEX_event_info1
{
    E_TEST_B,

};
enum TRACEX_event_info2
{
    E_TEST_C,
};
enum TRACEX_event_info3
{
    E_TEST_D,

};
enum TRACEX_event_info4
{
    E_TEST_E,

};

typedef struct
{
    uint32_t                            thread_pointer;     /* Thread pointer when the event happened*/
    uint32_t                            thread_priority;    /* Thread priority */
    enum TRACEX_event_id_t              event_id;           /* Event ID of the event */
    uint32_t                            time_stamp;         /* Timestamp when the event happened*/
    enum TRACEX_event_info1             info1;              /* Info 1 of the event */
    enum TRACEX_event_info2             info2;              /* Info 2 of the event */
    enum TRACEX_event_info3             info3;              /* Info 3 of the event */
    enum TRACEX_event_info4             info4;              /* Info 4 of the event */


} TRACEX_event_t;

#endif