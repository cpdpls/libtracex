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
    TRACEX_DEBUG_OBJ("Available : %u\n", entry->available);
    TRACEX_DEBUG_OBJ("Type : %s\n", tracex_object_convert_type_to_string(entry->type));
    if (entry->type == TRACEX_OBJECT_TYPE_THREAD)
    {
        TRACEX_DEBUG_OBJ("Thread Priority : %u\n", entry->thread_priority);
    }
    else
    {
        TRACEX_DEBUG_OBJ("Reserved 1 : %u\n", entry->res1);
        TRACEX_DEBUG_OBJ("Reserved 2 : %u\n", entry->res2);

    }
    TRACEX_DEBUG_OBJ("Object Pointer : 0x%.4x\n", entry->pointer);
    tracex_debug_print_object_params(entry);
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

static void tracex_debug_print_object_params(const struct tracex_object *object)
{

    switch( object->type)
    {
        case TRACEX_OBJECT_TYPE_THREAD:
            TRACEX_DEBUG_OBJ("Stack Start : 0x%.4x\n", object->objectParams.thread.stack_start);
            TRACEX_DEBUG_OBJ("Stack size : %u\n", object->objectParams.thread.stack_size);
            break;

    case TRACEX_OBJECT_TYPE_TIMER:
            TRACEX_DEBUG_OBJ("Initial Ticks : %u\n", object->objectParams.timer.initial_ticks);
            TRACEX_DEBUG_OBJ("Rescheduled Ticks : %u\n", object->objectParams.timer.reschedule_ticks);
            break;

    case TRACEX_OBJECT_TYPE_QUEUE:
        TRACEX_DEBUG_OBJ("Queue Size : %u\n", object->objectParams.queue.queue_size);
        TRACEX_DEBUG_OBJ("Message Size : %u\n", object->objectParams.queue.message_size);
        break;

    case TRACEX_OBJECT_TYPE_SEMAPHORE:
        TRACEX_DEBUG_OBJ("Initial Instances : %u\n", object->objectParams.semaphore.initial_instances);
        break;

    case TRACEX_OBJECT_TYPE_MUTEX:
        TRACEX_DEBUG_OBJ("Inheritance Flag : %u\n", object->objectParams.mutex.inheritance_flag);
        break;
    
    case TRACEX_OBJECT_TYPE_BLOCK_POOL:
        TRACEX_DEBUG_OBJ("Total Blocks : %u\n", object->objectParams.blockPool.total_blocks);
        TRACEX_DEBUG_OBJ("Block Size : %u\n", object->objectParams.blockPool.block_size);
        break;

    case TRACEX_OBJECT_TYPE_BYTE_POOL:
        TRACEX_DEBUG_OBJ("Total Bytes : %u\n", object->objectParams.bytePool.total_bytes);
        break;

    case TRACEX_OBJECT_TYPE_MEDIA:
        TRACEX_DEBUG_OBJ("FAT Cache Size : %u\n", object->objectParams.media.fat_cache_size);
        TRACEX_DEBUG_OBJ("Sector Cache Size : %u\n", object->objectParams.media.sector_cache_size);
        break;

    case TRACEX_OBJECT_TYPE_IP:
        TRACEX_DEBUG_OBJ("Stack Start : %u\n", object->objectParams.ip.stack_start);
        TRACEX_DEBUG_OBJ("Stack size : %u\n", object->objectParams.ip.stack_size);
        break;

    case TRACEX_OBJECT_TYPE_PACKET_POOL:
        TRACEX_DEBUG_OBJ("Packet Size : %u\n", object->objectParams.packetPool.packet_size);
        TRACEX_DEBUG_OBJ("Number Of Packets : %u\n", object->objectParams.packetPool.number_of_packets);
        break;

    /*TODO: Show the IP Address in string format */
    case TRACEX_OBJECT_TYPE_TCP_SOCKET:
        TRACEX_DEBUG_OBJ("IP Address : %u\n", object->objectParams.tcpSocket.ip_addr);
        TRACEX_DEBUG_OBJ("Window Size : %u\n", object->objectParams.tcpSocket.window_size);
        break;
    case TRACEX_OBJECT_TYPE_UDP_SOCKET:
        TRACEX_DEBUG_OBJ("IP Address : %u\n", object->objectParams.udpSocket.ip_addr);
        TRACEX_DEBUG_OBJ("Max RX Queue : %u\n", object->objectParams.udpSocket.rx_queue_max);
        break;
    
    case TRACEX_OBJECT_TYPE_EVENT_FLAGS_GROUP:
    case TRACEX_OBJECT_TYPE_FILE:
        break;
    
    default:
        TRACEX_DEBUG_OBJ("Generic Param 1 : %u\n", object->objectParams.param_1);
        TRACEX_DEBUG_OBJ("Generic Param 2: %u\n", object->objectParams.param_2);
        break;

    }

}