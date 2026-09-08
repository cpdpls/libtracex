#include <assert.h>
#include <stdio.h>
#include "tracex_debug.h"
#include "tracex_header_int.h"
#include "tracex_core.h"

#define TRACEX_DEBUG(fmt, args...) fprintf(stdout, "TRACEX-DBG >> " fmt, ##args)

void TRACEX_print_user_header(const struct TRACEX_header_t *header)
{
    assert(header != NULL);

    TRACEX_DEBUG("| BEGIN USER HEADER |\n");
    TRACEX_DEBUG("Endianess : %s\n", header->endianess ? "BIG ENDIAN": "LITTLE ENDIAN");
    TRACEX_DEBUG("Time-Stamp mask : 0x%.4x\n", header->timer_mask);
    TRACEX_DEBUG("Object Size : %d\n", header->object_name_size);
    TRACEX_DEBUG("| END USER HEADER |\n\n");
    
}


void TRACEX_print_raw_header(const struct TRACEX_handler_t *handler)
{
    const struct tracex_raw_header_t *raw_header;

    assert(handler != NULL);
    assert(handler->raw_dump.header.header_parsed == 1);
    assert(handler->raw_dump.header.header_valid == 1);

    /* Assign the raw header pointer */
    raw_header = &handler->raw_dump.header.raw_hdr;

    TRACEX_DEBUG("| BEGIN RAW HEADER |\n"); 
    TRACEX_DEBUG("Id : %.4s (0x%.4x)\n", (uint8_t*)&raw_header->id, raw_header->id);
    TRACEX_DEBUG("Time-Stamp mask : 0x%.4x\n", raw_header->timer_valid_mask);
    TRACEX_DEBUG("Trace Base Address : 0x%.4x\n", raw_header->trace_base_address);
    TRACEX_DEBUG("Object Registry Start Pointer : 0x%.4x\n", raw_header->obj_registry_start_pointer);
    TRACEX_DEBUG("Reserved 1 : 0x%.2x\n", raw_header->res1);
    TRACEX_DEBUG("Object Registry Name Size : %d\n", raw_header->obj_registry_name_size);
    TRACEX_DEBUG("Object Registry End Pointer : 0x%.4x\n", raw_header->obj_registry_end_pointer);
    TRACEX_DEBUG("Buffer Start Pointer : 0x%.4x\n", raw_header->buff_start_pointer);
    TRACEX_DEBUG("Buffer End Pointer : 0x%.4x\n", raw_header->buff_end_pointer);
    TRACEX_DEBUG("Buffer Current Pointer : 0x%.4x\n", raw_header->buff_current_pointer);
    TRACEX_DEBUG("Reserved 2 : 0x%.4x\n", raw_header->res2);
    TRACEX_DEBUG("Reserved 3 : 0x%.4x\n", raw_header->res3);
    TRACEX_DEBUG("Reserved 4 : 0x%.4x\n", raw_header->res4);
    TRACEX_DEBUG("| END RAW HEADER |\n\n");

}