#ifndef __TRACEX_EVENT_H__
#define __TRACEX_EVENT_H__

#include <stdint.h>
#include "tracex_errno.h"


enum tracex_event_occurence_type
{
    TRACEX_EVENT_OCCURENCE_TYPE_THREAD,
    TRACEX_EVENT_OCCURENCE_TYPE_ISR,
    TRACEX_EVENT_OCCURENCE_TYPE_INITIALIZATION,
};

struct tracex_event_info_labels {
    const uint8_t *info1;
    const uint8_t *info2;
    const uint8_t *info3;
    const uint8_t *info4;
};

struct tracex_event
{
    enum tracex_event_occurence_type occurenceType;
    
    uint32_t    threadPointer;     /* Thread pointer of the thread running when this event happened */
    
    union 
    {
        uint32_t threadPriority;            /* If the event occured when a thread was running, this is it's priority*/
        uint32_t threadPointerBeforeIsr;    /* If the event occured during an ISR, it's the thread running before the ISR */

    } threadParams;

    uint32_t    eventId;           /* Event ID of the event */
    uint32_t    timeStamp;         /* Timestamp when the event happened. To be used alongside the time stamp mask */
    uint32_t    info1;              /* Info 1 of the event */
    uint32_t    info2;              /* Info 2 of the event */
    uint32_t    info3;              /* Info 3 of the event */
    uint32_t    info4;              /* Info 4 of the event */

    struct tracex_event_info_labels infoLabels;
    const uint8_t *eventLabel;
};

#endif