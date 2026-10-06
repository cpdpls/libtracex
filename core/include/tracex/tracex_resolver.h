#ifndef TRACEX_RESOLVER_H
#define TRACEX_RESOLVER_H

#include "tracex/tracex_errno.h"
#include "tracex/tracex_object.h"
#include "tracex/tracex_event.h"

/* Forward declaration */
typedef struct tracex_handler tracex_handler_t;

enum tracex_resolver_request_type {
    E_TRACEX_RESOLVER_OBJECT,
    E_TRACEX_RESOLVER_EVENT
};

typedef struct {
	struct tracex_object_labels objLabels;
	struct tracex_event_labels eventLabels;

} tracex_resolver_labels;

typedef tracex_resolver_labels (*tracexResolverGetlabel)(enum tracex_resolver_request_type, uint32_t label_id);

tracex_ret_t tracex_register_resolver_function(tracex_handler_t *handler, tracexResolverGetlabel resolverFunc);

tracex_ret_t tracex_refresh_resolver_labels(tracex_handler_t *handler);
#endif