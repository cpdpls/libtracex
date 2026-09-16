#ifndef __TRACEX_DEBUG_H__
#define __TRACEX_DEBUG_H__

#include "tracex_header.h"
#include "tracex_object.h"
#include "tracex_event.h"

void TRACEX_debug_print_raw_header(const struct tracex_header *header);
void TRACEX_debug_print_single_object(const struct tracex_object *entry);
void TRACEX_debug_print_objects(const struct tracex_handler *handler);
void TRACEX_debug_print_single_event(const struct tracex_event *entry);
void TRACEX_debug_print_events(const struct tracex_handler *handler);
#endif