#include <assert.h>
#include <stdio.h>
#include "tracex_debug.h"
#include "tracex_header_int.h"
#include "tracex_core.h"
#include "tracex_obj_int.h"

#define TRACEX_DEBUG_HDR(fmt, args...) fprintf(stdout, "TRACEX-HEADER >> " fmt, ##args)
#define TRACEX_DEBUG_OBJ(fmt, args...) fprintf(stdout, "TRACEX-OBJECT >> " fmt, ##args)

static void tracex_debug_print_object_params(struct tracex_object_entry_t *object);

void TRACEX_debug_print_user_header(const struct TRACEX_header_t *header)
{
    assert(header != NULL);

    TRACEX_DEBUG_HDR("| BEGIN USER HEADER |\n");
    TRACEX_DEBUG_HDR("Endianess : %s\n", header->endianess ? "BIG ENDIAN": "LITTLE ENDIAN");
    TRACEX_DEBUG_HDR("Time-Stamp mask : 0x%.4x\n", header->timer_mask);
    TRACEX_DEBUG_HDR("Object Size : %u\n", header->object_name_size);
    TRACEX_DEBUG_HDR("| END USER HEADER |\n\n");
    
}


void TRACEX_debug_print_raw_header(const struct TRACEX_handler_t *handler)
{
    const struct tracex_raw_header_t *raw_header;

    assert(handler != NULL);
    assert(handler->raw_dump.header.header_parsed == 1);
    assert(handler->raw_dump.header.header_valid == 1);

    /* Assign the raw header pointer */
    raw_header = &handler->raw_dump.header.raw_hdr;

    TRACEX_DEBUG_HDR("| BEGIN RAW HEADER |\n"); 
    TRACEX_DEBUG_HDR("Id : %.4s (0x%.4x)\n", (uint8_t*)&raw_header->id, raw_header->id);
    TRACEX_DEBUG_HDR("Time-Stamp mask : 0x%.4x\n", raw_header->timer_valid_mask);
    TRACEX_DEBUG_HDR("Trace Base Address : 0x%.4x\n", raw_header->trace_base_address);
    TRACEX_DEBUG_HDR("Object Registry Start Pointer : 0x%.4x\n", raw_header->obj_registry_start_pointer);
    TRACEX_DEBUG_HDR("Reserved 1 : 0x%.2x\n", raw_header->res1);
    TRACEX_DEBUG_HDR("Object Registry Name Size : %u\n", raw_header->obj_registry_name_size);
    TRACEX_DEBUG_HDR("Object Registry End Pointer : 0x%.4x\n", raw_header->obj_registry_end_pointer);
    TRACEX_DEBUG_HDR("Buffer Start Pointer : 0x%.4x\n", raw_header->buff_start_pointer);
    TRACEX_DEBUG_HDR("Buffer End Pointer : 0x%.4x\n", raw_header->buff_end_pointer);
    TRACEX_DEBUG_HDR("Buffer Current Pointer : 0x%.4x\n", raw_header->buff_current_pointer);
    TRACEX_DEBUG_HDR("Reserved 2 : 0x%.4x\n", raw_header->res2);
    TRACEX_DEBUG_HDR("Reserved 3 : 0x%.4x\n", raw_header->res3);
    TRACEX_DEBUG_HDR("Reserved 4 : 0x%.4x\n", raw_header->res4);
    TRACEX_DEBUG_HDR("| END RAW HEADER |\n\n");

}   

void TRACEX_debug_print_objects(const struct TRACEX_handler_t *handler)
{
    struct tracex_object_entry_t *entry;
    uint64_t object_counter;

    assert(handler != NULL);
    assert(handler->raw_dump.objs.tot_object_count > 0);

    object_counter = 0;
    tracex_list_for_each_entry(entry, &handler->raw_dump.objs.obj_list, node)
    {
        TRACEX_DEBUG_OBJ("| BEGIN OBJECT ENTRY (%u) |\n", object_counter);
        TRACEX_DEBUG_OBJ("Available : %u\n", entry->obj.available);
        TRACEX_DEBUG_OBJ("Type : %s\n", TRACEX_objectTypeToString(entry->obj.type));
        if (entry->obj.type == TRACEX_OBJECT_TYPE_THREAD)
        {
            TRACEX_DEBUG_OBJ("Thread Priority : %u\n", entry->obj.thread_priority);
        }
        else
        {
            TRACEX_DEBUG_OBJ("Reserved 1 : %u\n", entry->obj.res1);
            TRACEX_DEBUG_OBJ("Reserved 2 : %u\n", entry->obj.res2);

        }
        TRACEX_DEBUG_OBJ("Object Pointer : 0x%.4x\n", entry->obj.pointer);
        tracex_debug_print_object_params(entry);
        TRACEX_DEBUG_OBJ("Name : %.32s\n", entry->obj.name);
        TRACEX_DEBUG_OBJ("| END OBJECT ENTRY (%u) |\n\n", object_counter++);
    }



}


static void tracex_debug_print_object_params(struct tracex_object_entry_t *object)
{

    switch( object->obj.type)
    {
        case TRACEX_OBJECT_TYPE_THREAD:
            TRACEX_DEBUG_OBJ("Stack Start : 0x%.4x\n", object->obj.objectParams.thread.stack_start);
            TRACEX_DEBUG_OBJ("Stack size : %u\n", object->obj.objectParams.thread.stack_size);
            break;

    case TRACEX_OBJECT_TYPE_TIMER:
            TRACEX_DEBUG_OBJ("Initial Ticks : %u\n", object->obj.objectParams.timer.initial_ticks);
            TRACEX_DEBUG_OBJ("Rescheduled Ticks : %u\n", object->obj.objectParams.timer.reschedule_ticks);
            break;

    case TRACEX_OBJECT_TYPE_QUEUE:
        TRACEX_DEBUG_OBJ("Queue Size : %u\n", object->obj.objectParams.queue.queue_size);
        TRACEX_DEBUG_OBJ("Message Size : %u\n", object->obj.objectParams.queue.message_size);
        break;

    case TRACEX_OBJECT_TYPE_SEMAPHORE:
        TRACEX_DEBUG_OBJ("Initial Instances : %u\n", object->obj.objectParams.semaphore.initial_instances);
        break;

    case TRACEX_OBJECT_TYPE_MUTEX:
        TRACEX_DEBUG_OBJ("Inheritance Flag : %u\n", object->obj.objectParams.mutex.inheritance_flag);
        break;
    
    case TRACEX_OBJECT_TYPE_BLOCK_POOL:
        TRACEX_DEBUG_OBJ("Total Blocks : %u\n", object->obj.objectParams.blockPool.total_blocks);
        TRACEX_DEBUG_OBJ("Block Size : %u\n", object->obj.objectParams.blockPool.block_size);
        break;

    case TRACEX_OBJECT_TYPE_BYTE_POOL:
        TRACEX_DEBUG_OBJ("Total Bytes : %u\n", object->obj.objectParams.bytePool.total_bytes);
        break;

    case TRACEX_OBJECT_TYPE_MEDIA:
        TRACEX_DEBUG_OBJ("FAT Cache Size : %u\n", object->obj.objectParams.media.fat_cache_size);
        TRACEX_DEBUG_OBJ("Sector Cache Size : %u\n", object->obj.objectParams.media.sector_cache_size);
        break;

    case TRACEX_OBJECT_TYPE_IP:
        TRACEX_DEBUG_OBJ("Stack Start : %u\n", object->obj.objectParams.ip.stack_start);
        TRACEX_DEBUG_OBJ("Stack size : %u\n", object->obj.objectParams.ip.stack_size);
        break;

    case TRACEX_OBJECT_TYPE_PACKET_POOL:
        TRACEX_DEBUG_OBJ("Packet Size : %u\n", object->obj.objectParams.packetPool.packet_size);
        TRACEX_DEBUG_OBJ("Number Of Packets : %u\n", object->obj.objectParams.packetPool.number_of_packets);
        break;

    /*TODO: Show the IP Address in string format */
    case TRACEX_OBJECT_TYPE_TCP_SOCKET:
        TRACEX_DEBUG_OBJ("IP Address : %u\n", object->obj.objectParams.tcpSocket.ip_addr);
        TRACEX_DEBUG_OBJ("Window Size : %u\n", object->obj.objectParams.tcpSocket.window_size);
        break;
    case TRACEX_OBJECT_TYPE_UDP_SOCKET:
        TRACEX_DEBUG_OBJ("IP Address : %u\n", object->obj.objectParams.udpSocket.ip_addr);
        TRACEX_DEBUG_OBJ("Max RX Queue : %u\n", object->obj.objectParams.udpSocket.rx_queue_max);
        break;
    
    case TRACEX_OBJECT_TYPE_EVENT_FLAGS_GROUP:
    case TRACEX_OBJECT_TYPE_FILE:
        break;
    
    default:
        TRACEX_DEBUG_OBJ("Geneic Param 1 : %u\n", object->obj.objectParams.param_1);
        TRACEX_DEBUG_OBJ("Geneic Param 2: %u\n", object->obj.objectParams.param_2);
        break;

    }

}