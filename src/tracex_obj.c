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

static void convert_from_raw_to_user(struct tracex_object_entry *entry, uint16_t name_size);
static tracex_ret_t parse_incrementally(struct tracex_object_context *ctx, void *buffer, size_t buffer_len, size_t *consumed);
static tracex_ret_t process_object(struct tracex_object_context *ctx);
static tracex_ret_t alloc_new_object_entry(struct tracex_object_entry **object_ptr, uint16_t name_length);
static void destroy_object_entry(struct tracex_object_entry **object);

tracex_ret_t tracex_object_int_init(struct tracex_object_context *ctx)
{
    if (pthread_mutex_init(&ctx->mutex, NULL) != 0)
    {
        return TRACEX_INIT_FAILURE;
    }

    /* Set the initial state machine and assign the pointer to the total object registry size */
    ctx->fsm = E_OBJ_PARSE_OTHERS;

    tracex_list_init(&ctx->obj_list);

    return TRACEX_SUCCESS;
}

tracex_ret_t tracex_object_int_compute_registry_size(uint64_t *registry_size, uint32_t start, uint32_t stop, uint32_t name_size)
{
    tracex_ret_t status;
    uint64_t object_entries;

    object_entries = 0;
    if (stop == start)
    {
        status = TRACEX_OBJECT_REGISTRY_INVALID;
        goto handle_exit;
    }

    /* Compute the total possible objects inside the registry */
    object_entries = ((stop - start) / ((sizeof(struct tracex_object_int) - sizeof(uint8_t*)) + name_size));
    if (object_entries == 0)
    {
        status = TRACEX_OBJECT_REGISTRY_INVALID;
        goto handle_exit;
    }

    status = TRACEX_SUCCESS;

handle_exit:
    *registry_size = object_entries;
    return status;
}

tracex_ret_t tracex_object_int_parse(struct tracex_object_context *ctx, void *buffer, size_t buff_len, uint64_t *consumed)
{
    tracex_ret_t status;
    size_t bytes_left;
    size_t bytes_consumed;

    pthread_mutex_lock(&ctx->mutex);

    bytes_left = buff_len;

    
    *consumed = 0;

    while (bytes_left != 0)
    {
        bytes_consumed = 0;
        status = parse_incrementally(ctx, buffer, bytes_left, &bytes_consumed);

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
            ctx->curr_count++;

            status = process_object(ctx);

            if (status == TRACEX_ALLOC_FAILURE)
            {
                goto handle_exit;
            }
            
            if (status == TRACEX_SUCCESS)
            {
                ctx->tot_count++;
            }

            status = TRACEX_NEED_MORE;

            /* Check if we have parsed the whole object registry*/
            if (ctx->curr_count == ctx->registry_size)
            {
                /* Reset the current session total object registry count */
                ctx->curr_count = 0;

                status = TRACEX_SUCCESS;
                goto handle_exit;
            }

        }
        

        /*Adjust the loop counter and the buffer position with it's size */
        buffer += bytes_consumed;
        bytes_left -= bytes_consumed;

    }

handle_exit:
    pthread_mutex_unlock(&ctx->mutex);
    return status;


}

void tracex_object_int_destroy_list(struct tracex_object_context *ctx)
{
    struct tracex_object_entry *entry;
    struct tracex_object_entry *next;
    
    
    tracex_list_for_each_entry_safe(entry, next, &ctx->obj_list, node)
    {
        destroy_object_entry(&entry);
    }
    

}

tracex_ret_t tracex_object_int_iterator_init(struct tracex_object_context *ctx, TRACEX_object_iterator_t **iterator)
{
    tracex_ret_t status;
    struct tracex_object_entry *entry;
    uint64_t index;

    /*TODO: Check the return value. We don't want to continue if it failed ... */
    /* Lock the mutex to block the parser for a short moment */
    pthread_mutex_lock(&ctx->mutex);

    *iterator = (struct tracex_obj_iterator*)malloc(sizeof(struct tracex_obj_iterator));
    if (*iterator == NULL)
    {
        status = TRACEX_ALLOC_FAILURE;
        goto handle_exit;
    }

    (*iterator)->objects = malloc(sizeof(struct tracex_object*) * ctx->tot_count);
    if((*iterator)->objects == NULL)
    {
        status = TRACEX_ALLOC_FAILURE;
        goto handle_error;
    }

    /* Loop on the objects inside the list */
    index = 0;
    tracex_list_for_each_entry(entry, &ctx->obj_list, node)
    {
        /* Assign the object */
        (*iterator)->objects[index++] = &entry->usr_obj;
    }

    (*iterator)->count = ctx->tot_count;
    (*iterator)->index = 0;

    status = TRACEX_SUCCESS;
    goto handle_exit;
    
handle_error:
    tracex_object_int_iterator_end(iterator);

handle_exit:
    /* Release the mutex */
    pthread_mutex_unlock(&ctx->mutex);
    return status;

}

tracex_ret_t tracex_object_int_iterator_next(TRACEX_object_iterator_t *iterator, const struct tracex_object **object)
{
    tracex_ret_t status;
    /* Check for some non-sense and return an error in that case */

    if (iterator == NULL)
    {
        status = TRACEX_BAD_INPUT_PTR;
        goto handle_exit;

    }
    if (iterator->objects == NULL || iterator->index > iterator->count)
    {
        status = TRACEX_OBJ_ITER_INVALID;
        goto handle_exit;
    }

    if (iterator->index >= iterator->count)
    {
        return TRACEX_OBJ_ITER_END;
    }

    *object = iterator->objects[iterator->index++];

    status = TRACEX_SUCCESS;
handle_exit:
    return status;
}

void tracex_object_int_iterator_end(TRACEX_object_iterator_t **iterator)
{
    if (iterator != NULL)
    {
        if (*iterator != NULL)
        {
            if ((*iterator)->objects != NULL)
            {
                free((*iterator)->objects);
                (*iterator)->objects = NULL;
            }
            free(*iterator);
            *iterator = NULL;
        }
    }
}

static void convert_from_raw_to_user(struct tracex_object_entry *entry, uint16_t name_size)
{
    entry->usr_obj.available = entry->raw_obj.available;
    entry->usr_obj.type = entry->raw_obj.type;
    entry->usr_obj.res1 = entry->raw_obj.res1;
    entry->usr_obj.res2 = entry->raw_obj.res2;
    entry->usr_obj.pointer = entry->raw_obj.pointer;
    entry->usr_obj.objectParams.param_1 = entry->raw_obj.param_1;
    entry->usr_obj.objectParams.param_2 = entry->raw_obj.param_2;

    memcpy(entry->usr_obj.name, entry->raw_obj.name, name_size);

}
static tracex_ret_t parse_incrementally(struct tracex_object_context *ctx, void *buffer, size_t buffer_len, size_t *consumed)
{
    tracex_ret_t status;
    void *start_address;
    size_t bytes_to_copy;
    enum tracex_obj_fsm last_fsm;

    
    /* Keep track of the previous FSM */
    last_fsm = ctx->fsm;
    bytes_to_copy = 0;

    /* Check if this is a new object and try to allocate memory for it */
    if (ctx->curr_offset == 0 && ctx->fsm == E_OBJ_PARSE_OTHERS)
    {
        if ((status = alloc_new_object_entry(&ctx->current_entry, ctx->name_size)) != TRACEX_SUCCESS)
        {
            goto handle_exit;
        }
    }
    
    /* Are we at the beginning of the parsing ? (The whole struct without the object name) */
    if (ctx->fsm == E_OBJ_PARSE_OTHERS)
    {
        /* Start address is the start of the object struct + the previous offset */
        start_address = ((void*)&ctx->current_entry->raw_obj) + ctx->curr_offset;

        /* Check if the buffer length added with the previous offset is bigger than the object struct - the size of the object name pointer */
        if (buffer_len + ctx->curr_offset >= (sizeof(struct tracex_object_int) - sizeof(uint8_t*)))
        {
            /* Copy all the structure fields until the start of the object name pointer in the struct (last field) */
            bytes_to_copy = (sizeof(struct tracex_object_int) - sizeof(uint8_t*)) - ctx->curr_offset;

            /* Reset the offset for the next loop */
            ctx->curr_offset = 0;

            /* Change the state machine in order to parse the object name */
            ctx->fsm = E_OBJ_PARSE_NAME;

        }
        /* Otherwise it's a partial copy, it's not enough to copy a full struct */
        else
        {
            /* Copy the whole provided buffer */
            bytes_to_copy = buffer_len;

            /* Increment the offset for the next iteration or function call */
            ctx->curr_offset += bytes_to_copy;
        }

    }

    /* We reached the point where we need to copy the object name */
    else if (ctx->fsm == E_OBJ_PARSE_NAME)
    {

        /* Base address is the object name field inside the struct */
        start_address = ((void*)ctx->current_entry->raw_obj.name) + ctx->curr_offset;

        /* Check if the buffer length added with the previous offset is bigger than the object name length */
        if (buffer_len + ctx->curr_offset >= ctx->name_size)
        {
            /* We have enough bytes to copy the full object name */
            bytes_to_copy = ctx->name_size - ctx->curr_offset;

        }
        /* Otherwise it's a partial copy, it's not enough to copy the full struct */
        else
        {
            /* We can't copy the full name at once, so copy only what we can */
            bytes_to_copy = buffer_len;

        }
        /* Increment the offset for the next iteration or function call */
        ctx->curr_offset += bytes_to_copy;
    }

    /* Perform the copy */
    memcpy(start_address, buffer, bytes_to_copy);

    /* Check if we reached the end of the parsing of an object */
    if (last_fsm == E_OBJ_PARSE_NAME && ctx->fsm == E_OBJ_PARSE_NAME && ctx->curr_offset == ctx->name_size)
    {
        /* Reset the FSM in order to parse the object fields again on the next call */
        ctx->fsm = E_OBJ_PARSE_OTHERS;
        
        /* Reset the offset for the next call  */
        ctx->curr_offset = 0;
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

static tracex_ret_t process_object(struct tracex_object_context *ctx)
{
    tracex_ret_t status;
    struct tracex_object_entry *entry;

    if (ctx->current_entry->raw_obj.available == 1 ||
        (ctx->current_entry->raw_obj.type == TRACEX_OBJECT_TYPE_NOT_VALID ||
        ctx->current_entry->raw_obj.type > TRACEX_OBJECT_TYPE_MAX))
    {
        status = TRACEX_OBJECT_INVALID;
        goto handle_exit;

    }
    /* Loop on the current objects */
    tracex_list_for_each_entry(entry, &ctx->obj_list, node)
    {
        /* This might be a destroyed object in the parsing, just ignore it the new object entry then*/
        if (entry->raw_obj.pointer == ctx->current_entry->raw_obj.pointer)
        {
            status = TRACEX_OBJECT_DUPLICATE;
            goto handle_exit;
        }
        
    }

    /* Convert the raw objet to the user format */
    convert_from_raw_to_user(ctx->current_entry, ctx->name_size);

    /* Add the object to the list */
    tracex_list_insert(&ctx->current_entry->node, &ctx->obj_list);

    status = TRACEX_SUCCESS;

handle_exit:

    if (status == TRACEX_SUCCESS) {
        /* Call the user provided callback */
        if (ctx->user_callback != NULL)
            ctx->user_callback(&ctx->current_entry->usr_obj, status);
    } else {
        destroy_object_entry(&ctx->current_entry);
    }
    return status;

}

static tracex_ret_t alloc_new_object_entry(struct tracex_object_entry **object_ptr, uint16_t name_length)
{
    tracex_ret_t status;
    struct tracex_object_entry *tmp_entry;

    /* Allocate a new object */
    tmp_entry = (struct tracex_object_entry*)malloc(sizeof(struct tracex_object_entry));
    if(tmp_entry == NULL)
    {
        status = TRACEX_ALLOC_FAILURE;
        goto handle_error;
    }

    /* Zero out the struct for safety */
    memset(tmp_entry, 0, sizeof(struct tracex_object_entry));


    /* We can now allocate memory for the raw object name */
    tmp_entry->raw_obj.name = (uint8_t*)malloc(sizeof(uint8_t) * name_length);
    if (tmp_entry->raw_obj.name == NULL)
    {
        status = TRACEX_ALLOC_FAILURE;
        goto handle_error;
    }

    /* Do the same for the user object name pointer */
    tmp_entry->usr_obj.name = (uint8_t*)malloc(sizeof(uint8_t) * name_length);
    if (tmp_entry->usr_obj.name == NULL)
    {
        status = TRACEX_ALLOC_FAILURE;
        goto handle_error;
    }


    status = TRACEX_SUCCESS;
    goto handle_exit;

handle_error:
    destroy_object_entry(&tmp_entry);

handle_exit:
    *object_ptr = tmp_entry;
    return status;
}



static void destroy_object_entry(struct tracex_object_entry **object)
{
    if (object != NULL)
    {
        if (*object != NULL)
        {
            /* Free the object name */
            if ((*object)->raw_obj.name != NULL)
            {
                free((*object)->raw_obj.name);
                (*object)->raw_obj.name = NULL;
            }

            if ((*object)->usr_obj.name != NULL)
            {
                free((*object)->usr_obj.name);
                (*object)->usr_obj.name = NULL;
                
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

const uint8_t *tracex_object_convert_type_to_string(enum tracex_object_type type)
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