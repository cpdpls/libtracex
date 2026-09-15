#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "tracex_object.h"
#include "tracex_obj_int.h"
#include "tracex_errno.h"
#include "tracex_core.h"
#include "tracex_header_int.h"

const char  *tracex_object_type_strings[] =
{
    "INVALID",
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
    "RESERVED",
    "USB HOST STACK DEVICE",
    "USB HOST STACK INTERFACE",
    "USB HOST ENDPOINT",
    "USB HOST CLASS",
    "USB DEV",
    "USB DEV INTERFACE",
    "USB DEV ENDPOINT",
    "USB DEV CLASS",

};

static TRACEX_Ret_t alloc_new_object_entry(struct tracex_object_entry_t **obj);
static TRACEX_Ret_t parse_incrementally(struct tracex_object_dump_t *dump, void *buffer, size_t buffer_len, size_t *consumed);
static TRACEX_Ret_t init_dump(struct tracex_object_dump_t *dump);
static TRACEX_Ret_t process_object(struct tracex_object_dump_t *dump);

TRACEX_Ret_t tracex_object_check_registry_valid(uint32_t start, uint32_t stop, uint32_t obj_name_length)
{
    
    if (stop == start)
    {
        return TRACEX_OBJECT_REGISTRY_INVALID;
    }

    if (((stop - start) / ((sizeof(TRACEX_object_t) - sizeof(uint8_t*)) + obj_name_length)) == 0)
    {
        return TRACEX_OBJECT_REGISTRY_INVALID;
    }

    return TRACEX_SUCCESS;
}

uint64_t tracex_object_compute_registry_size(uint32_t start, uint32_t stop, uint32_t obj_name_length)
{
    return ((stop - start) / ((sizeof(struct tracex_obj_raw_t) - sizeof(uint8_t*)) + obj_name_length));
}

TRACEX_Ret_t tracex_init_objects(struct tracex_object_dump_t *obj_dump, struct tracex_header_dump_t *hdr_dump)
{
    if (pthread_mutex_init(&obj_dump->object_mutex, NULL) != 0)
    {
        return TRACEX_INIT_FAILURE;
    }
    obj_dump->hdr_dump_ptr = hdr_dump;
    obj_dump->obj_fsm = E_OBJ_PARSE_OTHERS;

    tracex_list_init(&obj_dump->obj_list);

    return TRACEX_SUCCESS;
}

TRACEX_Ret_t tracex_parse_objects(struct tracex_object_dump_t *obj_dump, void *buffer, size_t buff_len, uint64_t *consumed)
{
    TRACEX_Ret_t status;
    size_t bytes_left;
    size_t bytes_consumed;

    pthread_mutex_lock(&obj_dump->object_mutex);

    bytes_left = buff_len;

    
    *consumed = 0;

    while (bytes_left != 0)
    {
        bytes_consumed = 0;
        status = parse_incrementally(obj_dump, buffer, bytes_left, &bytes_consumed);

        /* Sanitize for an error */
        if (status != TRACEX_SUCCESS && status != TRACEX_NEED_MORE)
        {
            goto handle_exit;
        }

        /* Increment the number of bytes consumed for the caller */
        *consumed += bytes_consumed;

        /* We parsed a full object, let's now process it and add it to the list */
        if (status == TRACEX_SUCCESS)
        {

            /* Increment the number of object parsed for the current session */
            obj_dump->curr_object_count++;

            status = process_object(obj_dump);

            if (status == TRACEX_ALLOC_FAILURE)
            {
                goto handle_exit;
            }
            
            if (status == TRACEX_SUCCESS)
            {
                obj_dump->tot_object_count++;
            }

            status = TRACEX_NEED_MORE;

            /* Check if we have parsed the whole object registry*/
            if (obj_dump->curr_object_count == obj_dump->hdr_dump_ptr->object_registry_size)
            {
                /* Reset the current session total object registry count */
                obj_dump->curr_object_count = 0;

                status = TRACEX_SUCCESS;
                goto handle_exit;
            }

        }
        

        /*Adjust the loop counter and the buffer position with it's size */
        buffer += bytes_consumed;
        bytes_left -= bytes_consumed;

    }

handle_exit:
    pthread_mutex_unlock(&obj_dump->object_mutex);
    return status;


}
void tracex_destroy_object(struct tracex_object_entry_t **object)
{
    if (object != NULL)
    {
        if (*object != NULL)
        {
            /* Free the object name */
            if ((*object)->obj.name != NULL)
            {
                free((*object)->obj.name);
                (*object)->obj.name = NULL;
            }
            if ((*object)->node.next != NULL && (*object)->node.prev != NULL)
            {
                tracex_list_delete(&(*object)->node);
                (*object)->node.next = NULL;
                (*object)->node.prev = NULL;
            }

            free(*object);
            (*object) = NULL;
        }
    }
}

void tracex_destroy_object_list(struct tracex_object_dump_t *obj_dump)
{
    /*TODO: Change this function to get paramer a struct tracex_object_dump_t*/
    /* So rename it to tracex_destroy_object_dump */
    /* The goal is to iterate on the objects_list and the user list and free those */
    struct tracex_object_entry_t *entry;
    struct tracex_object_entry_t *next;
    
    if (&obj_dump->obj_list != NULL)
    {
        tracex_list_for_each_entry_safe(entry, next, &obj_dump->obj_list, node)
        {
            tracex_destroy_object(&entry);
        }
    }

    

}

static TRACEX_Ret_t alloc_new_object_entry(struct tracex_object_entry_t **obj)
{
    
    struct tracex_object_entry_t *tmp_obj;

    /* Allocate a new object */
    tmp_obj = (struct tracex_object_entry_t*)malloc(sizeof(struct tracex_object_entry_t));
    if(tmp_obj == NULL)
    {
        return TRACEX_ALLOC_FAILURE;
    }

    /* Zero out the struct for safety */
    memset(tmp_obj, 0, sizeof(struct tracex_object_entry_t));
    *obj = tmp_obj;
    return TRACEX_SUCCESS;

}

static TRACEX_Ret_t parse_incrementally(struct tracex_object_dump_t *dump, void *buffer, size_t buffer_len, size_t *consumed)
{
    TRACEX_Ret_t status;
    void *start_address;
    size_t bytes_to_copy;
    enum tracex_obj_fsm_t last_fsm;

    
    /* Keep track of the previous FSM */
    last_fsm = dump->obj_fsm;
    bytes_to_copy = 0;

    /* Check if this is a new object and try to allocate memory for it */
    if (dump->curr_obj_offset == 0 && dump->obj_fsm == E_OBJ_PARSE_OTHERS)
    {
        /* Zero out the structure */
        memset(&dump->current_raw_obj, 0, sizeof(struct tracex_obj_raw_t));
        dump->current_raw_obj.obj_name = (uint8_t*)malloc(sizeof(uint8_t) * dump->hdr_dump_ptr->raw_hdr.obj_registry_name_size);
        if (dump->current_raw_obj.obj_name == NULL)
        {
            status = TRACEX_ALLOC_FAILURE;
        }
    }
    
    /* Are we at the beginning of the parsing ? (The whole struct without the object name) */
    if (dump->obj_fsm == E_OBJ_PARSE_OTHERS)
    {
        /* Start address is the start of the object struct + the previous offset */
        start_address = (void*)&dump->current_raw_obj + dump->curr_obj_offset;

        /* Check if the buffer length added with the previous offset is bigger than the object struct - the size of the object name pointer */
        if (buffer_len + dump->curr_obj_offset >= (sizeof(struct tracex_obj_raw_t) - sizeof(uint8_t*)))
        {
            /* Copy all the structure fields until the start of the object name pointer in the struct (last field) */
            bytes_to_copy = (sizeof(struct tracex_obj_raw_t) - sizeof(uint8_t*)) - dump->curr_obj_offset;

            /* Reset the offset for the next loop */
            dump->curr_obj_offset = 0;

            /* Change the state machine in order to parse the object name */
            dump->obj_fsm = E_OBJ_PARSE_NAME;

        }
        /* Otherwise it's a partial copy, it's not enough to copy a full struct */
        else
        {
            /* Copy the whole provided buffer */
            bytes_to_copy = buffer_len;

            /* Increment the offset for the next iteration or function call */
            dump->curr_obj_offset += bytes_to_copy;
        }

    }

    /* We reached the point where we need to copy the object name */
    else if (dump->obj_fsm == E_OBJ_PARSE_NAME)
    {

        /* Base address is the object name field inside the struct */
        start_address = (void*) dump->current_raw_obj.obj_name + dump->curr_obj_offset;

        /* Check if the buffer length added with the previous offset is bigger than the object name length */
        if (buffer_len + dump->curr_obj_offset >= dump->hdr_dump_ptr->raw_hdr.obj_registry_name_size)
        {
            /* We have enough bytes to copy the full object name */
            bytes_to_copy = dump->hdr_dump_ptr->raw_hdr.obj_registry_name_size - dump->curr_obj_offset;

        }
        /* Otherwise it's a partial copy, it's not enough to copy the full struct */
        else
        {
            /* We can't copy the full name at once, so copy only what we can */
            bytes_to_copy = buffer_len;

        }
        /* Increment the offset for the next iteration or function call */
        dump->curr_obj_offset += bytes_to_copy;
    }

    /* Perform the copy */
    memcpy(start_address, buffer, bytes_to_copy);

    /* Check if we reached the end of the parsing of an object */
    if (last_fsm == E_OBJ_PARSE_NAME && dump->obj_fsm == E_OBJ_PARSE_NAME && dump->curr_obj_offset == dump->hdr_dump_ptr->raw_hdr.obj_registry_name_size)
    {
        /* Reset the FSM in order to parse the object fields again on the next call */
        dump->obj_fsm = E_OBJ_PARSE_OTHERS;
        
        /* Reset the offset for the next call  */
        dump->curr_obj_offset = 0;
        status = TRACEX_SUCCESS;
    }
    /* We need more bytes to parse an object */
    else
    {
        status = TRACEX_NEED_MORE;
    }
    
    
handle_exit:
    *consumed = bytes_to_copy;
    return status;

}

static TRACEX_Ret_t process_object(struct tracex_object_dump_t *dump)
{
    TRACEX_Ret_t status;
    struct tracex_object_entry_t *entry;
    struct tracex_object_entry_t *user_object;

    if ( dump->current_raw_obj.obj_available == 1 || (dump->current_raw_obj.obj_type == TRACEX_OBJECT_TYPE_NOT_VALID || dump->current_raw_obj.obj_type > TRACEX_OBJECT_TYPE_USB_DEV_CLASS))
    {
        status = TRACEX_OBJECT_INVALID;
        goto handle_error;

    }
    /* Loop on the current objects */
    tracex_list_for_each_entry(entry, &dump->obj_list, node)
    {
        /* This might be a destroyed object in the parsing, just ignore it the new object entry then*/
        if (entry->obj.pointer == dump->current_raw_obj.obj_pointer)
        {
            status = TRACEX_OBJECT_DUPLICATE;
            goto handle_error;
        }
        
    }

    /*TODO: If we fail, we need to decrement the number of parsed bytes */
    if (alloc_new_object_entry(&user_object) != TRACEX_SUCCESS)
    {
        status =  TRACEX_ALLOC_FAILURE;
        goto handle_error;
    }


    /*TODO: Assign the correct endianess */
    user_object->obj.available = dump->current_raw_obj.obj_available;
    user_object->obj.type = dump->current_raw_obj.obj_type;
    user_object->obj.thread_priority = ((dump->current_raw_obj.obj_res1 & 0x3) << 8) |
                                        dump->current_raw_obj.obj_res2;
    user_object->obj.pointer = dump->current_raw_obj.obj_pointer;
    user_object->obj.objectParams.param_1 = dump->current_raw_obj.obj_param_1;
    user_object->obj.objectParams.param_2 = dump->current_raw_obj.obj_param_2;
    user_object->obj.name = dump->current_raw_obj.obj_name;

    /* Add the object to the list */
    tracex_list_insert(&user_object->node, &dump->obj_list);

    status = TRACEX_SUCCESS;

    goto handle_exit;

handle_error:
    /* Free the allocated object name */
    free(dump->current_raw_obj.obj_name);
    dump->current_raw_obj.obj_name = NULL;

handle_exit:
    /* Call the user provided callback */
    if (dump->parserCallback != NULL)
        dump->parserCallback(&user_object->obj, status);
    return status;

}

TRACEX_Ret_t tracex_object_iterator_init(struct tracex_object_dump_t *obj_dump, TRACEX_object_iterator_t **iter)
{
    TRACEX_Ret_t status;
    struct tracex_object_entry_t *entry;
    uint64_t index;

    /*TODO: Check the return value. We don't want to continue if it failed ... */
    /* Lock the mutex to block the parser for a short moment */
    pthread_mutex_lock(&obj_dump->object_mutex);

    *iter = (TRACEX_object_iterator_t*)malloc(sizeof(TRACEX_object_iterator_t));
    if (*iter == NULL)
    {
        status = TRACEX_ALLOC_FAILURE;
        goto handle_exit;
    }

    (*iter)->objects = malloc(sizeof(TRACEX_object_t*) *obj_dump->tot_object_count);
    if((*iter)->objects == NULL)
    {
        status = TRACEX_ALLOC_FAILURE;
        goto handle_error;
    }

    /* Loop on the objects inside the list */
    index = 0;
    tracex_list_for_each_entry(entry, &obj_dump->obj_list, node)
    {
        /* Assign the object */
        (*iter)->objects[index++] = &entry->obj;
    }

    (*iter)->count = obj_dump->tot_object_count;
    (*iter)->index = 0;

    status = TRACEX_SUCCESS;
    goto handle_exit;
    
handle_error:
    tracex_object_iter_end(iter);

handle_exit:
    /* Release the mutex */
    pthread_mutex_unlock(&obj_dump->object_mutex);
    return status;

}

TRACEX_Ret_t tracex_object_iter_next(TRACEX_object_iterator_t *iter, const TRACEX_object_t **object)
{
    /* Check for some non-sense and return an error in that case */
    if (iter->objects == NULL || iter->index > iter->count)
    {
        return TRACEX_OBJ_ITER_INVALID;
    }

    if (iter->index >= iter->count)
    {
        return TRACEX_OBJ_ITER_END;
    }

    *object = iter->objects[iter->index++];

    return TRACEX_SUCCESS;
}

void tracex_object_iter_end(TRACEX_object_iterator_t **iter)
{
    if (iter != NULL)
    {
        if (*iter != NULL)
        {
            if ((*iter)->objects != NULL)
            {
                free((*iter)->objects);
                (*iter)->objects = NULL;
            }
            free(*iter);
            *iter = NULL;
        }
    }
}

const uint8_t *TRACEX_objectTypeToString(enum TRACEX_ObjectType type)
{
    const char *string_type;

    if (type >= TRACEX_OBJECT_TYPE_USB_DEV_CLASS)
    {
        string_type = tracex_object_type_strings[0];
    }
    else if (type >= TRACEX_OBJECT_TYPE_RESERVED && type < TRACEX_OBJECT_TYPE_USB_HOST_STACK_DEV)
    {
        string_type = tracex_object_type_strings[TRACEX_OBJECT_TYPE_RESERVED];
    }
    else
    {
        string_type = tracex_object_type_strings[type];
    }

    return string_type;

}