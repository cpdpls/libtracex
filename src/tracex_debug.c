#include <assert.h>
#include <stdio.h>
#include "tracex_debug.h"
#include "tracex_header_int.h"
#include "tracex_core.h"
#include "tracex_obj_int.h"
#include "tracex_object.h"
#include "tracex_event_int.h"

#define TRACEX_DEBUG_HDR(fmt, args...) fprintf(stdout, "TRACEX-HEADER >> " fmt, ##args)
#define TRACEX_DEBUG_OBJ(fmt, args...) fprintf(stdout, "TRACEX-OBJECT >> " fmt, ##args)
#define TRACEX_DEBUG_EVENT(fmt, args...) fprintf(stdout, "TRACEX-EVENT >> " fmt, ##args)


void TRACEX_debug_print_raw_header(const struct tracex_header *header)
{

    /* Assign the raw header pointer */

    TRACEX_DEBUG_HDR("| BEGIN RAW HEADER |\n"); 
    TRACEX_DEBUG_HDR("Id : %.4s (0x%.4x)\n", (uint8_t*)&header->id, header->id);
    TRACEX_DEBUG_HDR("Time-Stamp mask : 0x%.4x\n", header->timestamp_mask);
    TRACEX_DEBUG_HDR("Trace Base Address : 0x%.4x\n", header->trace_base_addr);
    TRACEX_DEBUG_HDR("Object Registry Start Pointer : 0x%.4x\n", header->obj_registry_start_ptr);
    TRACEX_DEBUG_HDR("Reserved 1 : 0x%.2x\n", header->res1);
    TRACEX_DEBUG_HDR("Object Registry Name Size : %u\n", header->obj_registry_name_size);
    TRACEX_DEBUG_HDR("Object Registry End Pointer : 0x%.4x\n", header->obj_registry_end_ptr);
    TRACEX_DEBUG_HDR("Buffer Start Pointer : 0x%.4x\n", header->event_buff_start_ptr);
    TRACEX_DEBUG_HDR("Buffer End Pointer : 0x%.4x\n", header->event_buff_end_ptr);
    TRACEX_DEBUG_HDR("Buffer Current Pointer : 0x%.4x\n", header->event_buff_curr_ptr);
    TRACEX_DEBUG_HDR("Reserved 2 : 0x%.4x\n", header->res2);
    TRACEX_DEBUG_HDR("Reserved 3 : 0x%.4x\n", header->res3);
    TRACEX_DEBUG_HDR("Reserved 4 : 0x%.4x\n", header->res4);
    TRACEX_DEBUG_HDR("| END RAW HEADER |\n\n");

}   

void TRACEX_debug_print_single_object(const struct tracex_object *entry)
{
    TRACEX_DEBUG_OBJ("---- %.32s ----\n", entry->name);
    TRACEX_DEBUG_OBJ("Available : %u\n", entry->available);
    TRACEX_DEBUG_OBJ("Type : %u\n", entry->type);
    TRACEX_DEBUG_OBJ("Reserved 1 : %u\n", entry->res1);
    TRACEX_DEBUG_OBJ("Reserved 2 : %u\n", entry->res2);
    TRACEX_DEBUG_OBJ("Object Pointer : 0x%.4x\n", entry->pointer);
    TRACEX_DEBUG_OBJ("Param 1 : %u\n", entry->param_1);
    TRACEX_DEBUG_OBJ("Param 2: %u\n", entry->param_2);
    TRACEX_DEBUG_OBJ("Name : %.32s\n\n", entry->name);
}


void TRACEX_debug_print_objects(const struct tracex_handler *handler)
{
    struct tracex_object_entry *entry;
    uint64_t object_counter;

    assert(handler != NULL);
    assert(handler->objs_ctx.tot_count > 0);

    object_counter = 0;
    tracex_list_for_each_entry(entry, &handler->objs_ctx.obj_list, node)
    {
        TRACEX_DEBUG_OBJ("| BEGIN OBJECT ENTRY (%lu) |\n", object_counter);
        TRACEX_debug_print_single_object(&entry->obj);
        TRACEX_DEBUG_OBJ("| END OBJECT ENTRY (%lu) |\n\n", object_counter++);
    }

}


void TRACEX_debug_print_single_event(const struct tracex_event *entry)
{
    TRACEX_DEBUG_EVENT("Event ID : %u\n", entry->event_id);
    TRACEX_DEBUG_EVENT("Thread Pointer : 0x%.4x\n", entry->thread_pointer);
    TRACEX_DEBUG_EVENT("Thread Priority : 0x%.4x\n", entry->thread_priority);
    TRACEX_DEBUG_EVENT("Time-Stamp : %u\n", entry->thread_priority);
    TRACEX_DEBUG_EVENT("Info 1 : %u\n", entry->info1);
    TRACEX_DEBUG_EVENT("Info 2 : %u\n", entry->info2);
    TRACEX_DEBUG_EVENT("Info 3 : %u\n", entry->info3);
    TRACEX_DEBUG_EVENT("Info 4 : %u\n", entry->info4);

}
void TRACEX_debug_print_events(const struct tracex_handler *handler)
{
    struct tracex_event_entry *entry;
    uint64_t event_counter;

    assert(handler != NULL);
    assert(handler->objs_ctx.tot_count > 0);

    event_counter = 0;
    tracex_list_for_each_entry(entry, &handler->event_ctx.event_list, node)
    {
        TRACEX_DEBUG_EVENT("| BEGIN EVENT ENTRY (%lu) |\n", event_counter);
        TRACEX_debug_print_single_event(&entry->event);
        TRACEX_DEBUG_EVENT("| END EVENT ENTRY (%lu) |\n\n", event_counter++);
    }

}