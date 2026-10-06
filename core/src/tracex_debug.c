#include <assert.h>
#include <stdio.h>
#include "tracex/tracex_debug.h"
#include "tracex_header_int.h"
#include "tracex_core.h"
#include "tracex/tracex_object.h"
#include "tracex_event_int.h"
#include "tracex/tracex.h"


#define TRACEX_DEBUG_HDR(fmt, args...) fprintf(stdout, "TRACEX-HEADER >> " fmt, ##args)
#define TRACEX_DEBUG_OBJ(fmt, args...) fprintf(stdout, "TRACEX-OBJECT >> " fmt, ##args)
#define TRACEX_DEBUG_EVENT(fmt, args...) fprintf(stdout, "TRACEX-EVENT >> " fmt, ##args)

static void tracex_debug_print_object_params(const struct tracex_object *object);


void TRACEX_debug_print_user_header(struct tracex_header *user_hdr)
{
    TRACEX_DEBUG_HDR("| BEGIN USER HEADER |\n"); 
    TRACEX_DEBUG_HDR("Id : %.4s (0x%.4x)\n", (uint8_t*)&user_hdr->Id, user_hdr->Id);
    TRACEX_DEBUG_HDR("Time-Stamp mask : 0x%.4x\n", user_hdr->timeStampMask);
    TRACEX_DEBUG_HDR("Object Registry Name Size : %u\n", user_hdr->obj_registry_name_size);
    TRACEX_DEBUG_HDR("| END USER HEADER |\n\n");

}

void TRACEX_debug_print_single_object(const struct tracex_object *entry)
{

    TRACEX_DEBUG_OBJ("---- %.32s ----\n", entry->name);
    TRACEX_DEBUG_OBJ("Type : %u\n", entry->type);
    TRACEX_DEBUG_OBJ("TypeName : %s\n", entry->labels.objectTypeName);
    if (entry->type == 1) {
	    TRACEX_DEBUG_OBJ("Thread Priority : %u\n", entry->thread_priority);
    } else {
	    TRACEX_DEBUG_OBJ("Reserved 1 : %u\n", entry->thread_priority);
	    TRACEX_DEBUG_OBJ("Reserved 2 : %u\n", entry->thread_priority);
    }
    TRACEX_DEBUG_OBJ("Object Pointer : 0x%.4x\n", entry->pointer);
    TRACEX_DEBUG_OBJ("%s : 0x%.4x\n", entry->labels.param1, entry->params.param1);
    TRACEX_DEBUG_OBJ("%s : 0x%.4x\n", entry->labels.param2, entry->params.param2);
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
    TRACEX_DEBUG_EVENT("Event Name : %s\n", entry->labels.event_name);
    TRACEX_DEBUG_EVENT("Thread Pointer : 0x%.4x\n", entry->threadPointer);

    if (entry->occurenceType == TRACEX_EVENT_OCCURENCE_TYPE_ISR)
        TRACEX_DEBUG_EVENT("Thread running before ISR : 0x%.4x\n", entry->threadPriority);
    
    if (entry->occurenceType == TRACEX_EVENT_OCCURENCE_TYPE_THREAD)
        TRACEX_DEBUG_EVENT("Thread Priority : %u\n", entry->threadPriority);
    
    TRACEX_DEBUG_EVENT("Time-Stamp : %u\n", entry->timeStamp);
    TRACEX_DEBUG_EVENT("%s : 0x%.4x\n", entry->labels.info1,  entry->rawInfos.info1);
    TRACEX_DEBUG_EVENT("%s : 0x%.4x\n", entry->labels.info2, entry->rawInfos.info2);
    TRACEX_DEBUG_EVENT("%s : 0x%.4x\n", entry->labels.info3, entry->rawInfos.info3);
    TRACEX_DEBUG_EVENT("%s : 0x%.4x\n\n", entry->labels.info4, entry->rawInfos.info4);

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