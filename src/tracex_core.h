#ifndef __TRACEX_CORE_H__
#define __TRACEX_CORE_H__

#include <stdint.h>
#include "tracex_errno.h"
#include "tracex_header_int.h"
#include "tracex_obj_int.h"
#include "tracex_event_int.h"
#include "tracex_list.h"
#include "tracex.h"

enum tracex_state_t
{
    E_HEADER_PHASE,
    E_OBJECT_PHASE,
    E_EVENT_PHASE,s
};

struct tracex_struct_t
{
    struct tracex_hdr_entry_t   header;
    struct tracex_list          obj_list;
    struct tracex_list          event_list;
    uint64_t                    object_count;
    uint64_t                    event_count;
};

struct TRACEX_handler_t
{
    struct tracex_struct_t      parsed_orig_dump;
    uint8_t                     header_valid;
    size_t                      dump_size;
    struct tracex_list          node;
    enum tracex_state_t         state;
};

#endif