#include <stdint.h>
#include <stdlib.h>
#include "tracex_object.h"
#include "tracex_obj_str.h"

const uint8_t *empty_string = "None";

const uint8_t *tracex_object_type_strings[] =
{
    "THREAD",
    "TIMER",
    "QUEUE",
    "SEMAPHORE",
    "MUTEX",
    "EVENT FLAGS GROUP",
    "BLOCK POOL",
    "BYTE POOL",
    "MEDIA",
    "FILE",
    "IP",
    "PACKET POOL",
    "TCP SOCKET",
    "UDP SOCKET",
    "USB HOST STACK DEVICE",
    "USB HOST STACK INTERFACE",
    "USB HOST ENDPOINT",
    "USB HOST CLASS",
    "USB DEVICE",
    "USB DEVICE INTERFACE",
    "USB DEVICE ENDPOINT",
    "USB DEVICE CLASS",

};

const struct tracex_object_params_str tracex_param_strings[] = 
{
    [TRACEX_OBJECT_TYPE_THREAD] = {"Stack Start", "Stack Size"},
    [TRACEX_OBJECT_TYPE_TIMER] = {"Initial Ticks", "Reschedule Ticks"},
    [TRACEX_OBJECT_TYPE_QUEUE] = {"Queue Size", "Message Size"},
    [TRACEX_OBJECT_TYPE_SEMAPHORE] = {"Initial Instances", 0},
    [TRACEX_OBJECT_TYPE_MUTEX] = {"Inheritance Flag", 0},
    [TRACEX_OBJECT_TYPE_EVENT_FLAGS_GROUP] = {0, 0},
    [TRACEX_OBJECT_TYPE_BLOCK_POOL] = {"Total Blocks", "Block Size"},
    [TRACEX_OBJECT_TYPE_BYTE_POOL] = {"Total Bytes", 0},
    [TRACEX_OBJECT_TYPE_MEDIA] = {"Fat Cache Size", "Sector Cache Size"},
    [TRACEX_OBJECT_TYPE_FILE] = {0, 0},
    [TRACEX_OBJECT_TYPE_IP] = {"Stack Start", "Stack Size"},
    [TRACEX_OBJECT_TYPE_PACKET_POOL] = {"Packet Size", "Packets Count"},
    [TRACEX_OBJECT_TYPE_TCP_SOCKET] = {"Ip Address", "Window Size"},
    [TRACEX_OBJECT_TYPE_UDP_SOCKET] = {"Ip Address", "RX Queue Max"},
};

const uint8_t *tracex_object_type_to_str(enum tracex_object_type type)
{
    const uint8_t *string;
    uint8_t index;

    /* There is no need to perfom any input checking as the processing function that calls this */
    /* Already should have done the job. If not, this is a bug ! */
    /* Only checks to do is in order to get the appropriate label */

    /* Compensate for the INVALID type which is the index 0 and is not part of the strings */
    index = type - 1;

    /* If the type is before the reserved area, dereference at index directly */
    if (type >= TRACEX_OBJECT_TYPE_THREAD && type <= TRACEX_OBJECT_TYPE_UDP_SOCKET) {
        string = tracex_object_type_strings[index];
    }

    /* Otherwise, the type is past the reserved area, so add the correct offset to it */
    else
    {   
        /* Compensate for the reserved area */
        index -= TRACXEX_OBJ_TYPE_RESERVED_SIZE;

        string = tracex_object_type_strings[index];
    }

    return string;
}

struct tracex_object_params_str tracex_object_param_to_str(enum tracex_object_type type)
{
    struct tracex_object_params_str params;


    /* As if today, the obejct parameters exists from the THREAD type until UDP Socket Type */
    /* So check for this range */
    if (type >= TRACEX_OBJECT_TYPE_THREAD && type <= TRACEX_OBJECT_TYPE_UDP_SOCKET) {

        /* Check if the param 1 is not null and assign it, otherwise, assign the empty string */
        if (tracex_param_strings[type].param1_label != NULL)
            params.param1_label = tracex_param_strings[type].param1_label;
        else
            params.param1_label = empty_string;


        /* Do the same for the param 2 */
        if (type >= TRACEX_OBJECT_TYPE_THREAD && type <= TRACEX_OBJECT_TYPE_UDP_SOCKET) {

            /* Check if the param 1 is not null and assign it, otherwise, assign the empty string */
            if (tracex_param_strings[type].param2_label != NULL)
                params.param2_label = tracex_param_strings[type].param2_label;
            else
                params.param2_label = empty_string;
            
        }

    } else {    /* If thats not the case, we need to point both param 1 and param 2 to the empty string */
        params.param1_label = empty_string;
        params.param2_label = empty_string;

    }

    return params;




}