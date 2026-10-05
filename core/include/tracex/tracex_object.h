#ifndef TRACEX_OBJECT_H
#define TRACEX_OBJECT_H

#include <stdint.h>
#include "tracex_errno.h"


typedef struct tracex_obj_iterator TRACEX_object_iterator_t;

struct tracex_object_labels {
    const char *objectTypeName;

    const char *param1;
    const char *param2;
};

struct tracex_resolver_obj_engine{
	int (*init_engine)(const char *path);
	struct tracex_object_labels (*get_object_labels)(uint32_t object_type);
	void (*deinit_engine)(void);
	const char *file_path;
};

struct tracex_object
{
    uint32_t type;
    uint16_t thread_priority;

    uint32_t pointer;

    struct {
	    uint32_t param1;
	    uint32_t param2;
    } params;

    uint8_t     *name;

    struct tracex_object_labels labels;
};

#endif
