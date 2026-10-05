#ifndef TRACEX_HEADER_H
#define TRACEX_HEADER_H

#include <stdint.h>
#include "tracex_errno.h"

typedef struct tracex_handler tracex_handler;       /* Forward declaration */

enum tracex_dump_endianess
{
    E_TRACEX_LITTLE_ENDIAN,
    E_TRACEX_BIG_ENDIAN
};


struct tracex_header {
    uint32_t Id;
    uint32_t timeStampMask;
    uint16_t obj_registry_name_size;
};

#endif
