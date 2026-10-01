#include <assert.h>
#include <stdio.h>
#include "tracex/tracex_debug.h"
#include "tracex_header_int.h"
#include "tracex_core.h"
#include "tracex/tracex_object.h"
#include "tracex_event_int.h"

#define TRACEX_DEBUG_HDR(fmt, args...) fprintf(stdout, "TRACEX-HEADER >> " fmt, ##args)
#define TRACEX_DEBUG_OBJ(fmt, args...) fprintf(stdout, "TRACEX-OBJECT >> " fmt, ##args)
#define TRACEX_DEBUG_EVENT(fmt, args...) fprintf(stdout, "TRACEX-EVENT >> " fmt, ##args)

static void tracex_debug_print_object_params(const struct tracex_object *object);

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
    TRACEX_DEBUG_OBJ("Type : %u\n", entry->type);
    if (entry->type == 1)
    {
        TRACEX_DEBUG_OBJ("Thread Priority : %u\n", entry->thread_priority);
    }
    else
    {
        TRACEX_DEBUG_OBJ("Reserved 1 : %u\n", entry->res1);
        TRACEX_DEBUG_OBJ("Reserved 2 : %u\n", entry->res2);

    }
    TRACEX_DEBUG_OBJ("Object Pointer : 0x%.4x\n", entry->pointer);
    // TRACEX_DEBUG_OBJ("%s : 0x%.4x\n", entry->param1Label, entry->params.raw.param1);
    // TRACEX_DEBUG_OBJ("%s : 0x%.4x\n", entry->param2Label, entry->params.raw.param2);
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
        TRACEX_debug_print_single_object(&entry->usr_obj);
        TRACEX_DEBUG_OBJ("| END OBJECT ENTRY (%lu) |\n\n", object_counter++);
    }

}


void TRACEX_debug_print_single_event(const struct tracex_event *entry)
{

    switch (entry->occurenceType) {
        case TRACEX_EVENT_OCCURENCE_TYPE_INITIALIZATION:
            TRACEX_DEBUG_EVENT("Event happened during initialization: (0x%.4x)\n", entry->threadPointer);
            break;
        case TRACEX_EVENT_OCCURENCE_TYPE_THREAD:
            TRACEX_DEBUG_EVENT("Event happened inside a thread : (0x%.4x)\n", entry->threadPointer);
            break;
        case TRACEX_EVENT_OCCURENCE_TYPE_ISR:
            TRACEX_DEBUG_EVENT("Event happened during an ISR : (0x%.4x)\n", entry->threadPointer);
            break;

    }

    TRACEX_DEBUG_EVENT("Event ID : %u\n", entry->eventId);
    TRACEX_DEBUG_EVENT("Thread Pointer : 0x%.4x\n", entry->threadPointer);

    if (entry->occurenceType == TRACEX_EVENT_OCCURENCE_TYPE_ISR)
        TRACEX_DEBUG_EVENT("Thread running before ISR : 0x%.4x\n", entry->threadPointerBeforeIsr);
    
    if (entry->occurenceType == TRACEX_EVENT_OCCURENCE_TYPE_THREAD)
        TRACEX_DEBUG_EVENT("Thread Priority : %u\n", entry->threadPriority);
    
    TRACEX_DEBUG_EVENT("Time-Stamp : %u\n", entry->timeStamp);
    // TRACEX_DEBUG_EVENT("%s : 0x%.4x\n", entry->info1Label,  entry->rawInfos.info1);
    // TRACEX_DEBUG_EVENT("%s : 0x%.4x\n", entry->info2Label, entry->rawInfos.info2);
    // TRACEX_DEBUG_EVENT("%s : 0x%.4x\n", entry->info3Label, entry->rawInfos.info3);
    // TRACEX_DEBUG_EVENT("%s : 0x%.4x\n\n", entry->info4Label, entry->rawInfos.info4);

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

static void tracex_debug_print_object_params(const struct tracex_object *object)
{



}