#ifndef __TRACEX_DEBUG_H__
#define __TRACEX_DEBUG_H__

#include "tracex_header.h"
#include "tracex_object.h"
#include "tracex_event.h"

void TRACEX_debug_print_user_header(const struct TRACEX_header_t *header);
void TRACEX_debug_print_raw_header(const struct TRACEX_handler_t *handler);
void TRACEX_debug_print_single_object(const TRACEX_object_t *entry);
void TRACEX_debug_print_objects(const struct TRACEX_handler_t *handler);
void TRACEX_debug_print_single_event(const TRACEX_event_t *entry);
void TRACEX_debug_print_events(const struct TRACEX_handler_t *handler);
#endif