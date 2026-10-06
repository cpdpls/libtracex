#ifndef TRACEX_EVENT_H
#define TRACEX_EVENT_H

#include <stdint.h>
#include "tracex_errno.h"


enum tracex_event_occurence_type
{
    TRACEX_EVENT_OCCURENCE_TYPE_THREAD,
    TRACEX_EVENT_OCCURENCE_TYPE_ISR,
    TRACEX_EVENT_OCCURENCE_TYPE_INITIALIZATION
};

struct tracex_event_labels{
	const char *event_name;
	
	const char *info1;
	const char *info2;
	const char *info3;
	const char *info4;

};

struct tracex_event {
	enum tracex_event_occurence_type occurenceType;

	uint32_t threadPointer; /* Thread pointer of the thread running when this event happened */

	uint32_t threadPriority; /* If the event occured when a thread was running, this is it's priority*/

	uint32_t eventId; /* Event ID of the event */
	uint32_t timeStamp; /* Timestamp when the event happened. To be used alongside the time stamp mask */

	struct {
		uint32_t info1; /* Info 1 of the event */
		uint32_t info2; /* Info 2 of the event */
		uint32_t info3; /* Info 3 of the event */
		uint32_t info4; /* Info 4 of the event */
	} rawInfos;

	struct tracex_event_labels labels;
};


#endif
