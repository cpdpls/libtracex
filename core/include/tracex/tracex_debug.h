#ifndef TRACEX_DEBUG_H
#define TRACEX_DEBUG_H

#include "tracex_header.h"
#include "tracex_object.h"
#include "tracex_event.h"

/* Forward declaration */
typedef struct tracex_handler tracex_handler_t;

void TRACEX_debug_print_user_header(struct tracex_header *user_hdr);
void TRACEX_debug_print_single_object(const struct tracex_object *entry);
void TRACEX_debug_print_objects(const struct tracex_handler *handler);
void TRACEX_debug_print_single_event(const struct tracex_event *entry);
void TRACEX_debug_print_events(const struct tracex_handler *handler);
#endif
