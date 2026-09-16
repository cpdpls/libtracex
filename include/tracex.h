#ifndef __TRACEX_H__
#define __TRACEX_H__

#include <stdint.h>
#include <stddef.h>
#include "tracex_errno.h"
#include "tracex_header.h"
#include "tracex_event.h"
#include "tracex_object.h"

typedef struct tracex_handler tracex_handler_t;


struct tracex_callbacks
{
    void (*on_header_parsed)(struct tracex_header *header, tracex_ret_t status);
    void (*on_object_parsed)(struct tracex_object *object, tracex_ret_t status);
    void (*on_event_parsed)(struct tracex_event *event, tracex_ret_t status);
};


void tracex_init(void);

tracex_ret_t tracex_create_new_handler(tracex_handler_t **new_handler);
tracex_ret_t tracex_register_callbacks(tracex_handler_t *handler, struct tracex_callbacks *callbacks);
tracex_ret_t tracex_parse(tracex_handler_t *handler, void *buffer, size_t buffer_length);
void tracex_destroy_handler(tracex_handler_t **handler);


#endif