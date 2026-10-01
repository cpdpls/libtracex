#ifndef __TRACEX_CORE_H__
#define __TRACEX_CORE_H__

#include <stdint.h>

#include "tracex/tracex_errno.h"
#include "tracex/tracex.h"
#include "tracex_header_int.h"
#include "tracex_obj_int.h"
#include "tracex_event_int.h"
#include "tracex_list.h"

#define TO_HANDLER(x) ((struct tracex_handler *)x)

enum tracex_parsing_state_t {
	E_HEADER_PHASE, /* Header phase FSM */
	E_OBJECT_PHASE, /* Object phase FSM */
	E_EVENT_PHASE, /* Event phase FSM */
};

struct tracex_handler {
	struct tracex_header_context hdr_ctx;
	struct tracex_object_context objs_ctx;
	struct tracex_event_context event_ctx;
	size_t raw_bytes_count; /* Total number of bytes of the dump */
	struct tracex_list node; /* Pointers to the next TRACEX_handle_t node */
	enum tracex_parsing_state_t state; /* Saved Finite State Machine between calls */
	struct tracex_callbacks user_callbacks; /* Holds the pointer to the user provided callbacks */
};

#endif