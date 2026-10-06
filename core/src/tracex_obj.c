#include <stdlib.h>
#include <string.h>

#include "tracex/tracex_object.h"
#include "tracex/tracex_errno.h"
#include "tracex_obj_int.h"
#include "tracex_core.h"
#include "tracex_header_int.h"

/* In case a resolver engine is not registed or, the engine returns an error,
 * This string will be assigned to the object param string
 */
static char *default_resolver_invalid_params_str = "Not valid"; 

/* In case a resolver engine is not registed or, the engine returns an error,
 * This string will be assigned to the object name
 */
static char *default_resolver_invalid_obj_str = "Invalid";

static void convert_from_raw_to_user(struct tracex_object_context *ctx, struct tracex_object_raw *raw, struct tracex_object *user);
static void resolve_labels(struct tracex_object_context *ctx, struct tracex_object *object);
static tracex_ret_t parse_incrementally(struct tracex_object_context *ctx, void *buffer, size_t buffer_len, size_t *consumed);
static tracex_ret_t process_object(struct tracex_object_context *ctx);
static tracex_ret_t alloc_new_object_entry(struct tracex_object_entry **object_ptr, uint16_t name_length);

static void destroy_object_list(struct tracex_object_context *ctx);
static void destroy_object_entry(struct tracex_object_entry **object);

tracex_ret_t tracex_object_int_init(struct tracex_object_context *ctx)
{
    /* Set the initial state machine and assign the pointer to the total object registry size */
    ctx->fsm = E_OBJ_PARSE_OTHERS;

    tracex_list_init(&ctx->obj_list);

    return TRACEX_SUCCESS;
}

tracex_ret_t tracex_object_int_compute_registry_size(uint64_t *registry_size, uint32_t start, uint32_t stop, uint32_t name_size)
{
    tracex_ret_t status;
    uint64_t object_entries = 0;

    if (stop == start)
    {
        status = TRACEX_OBJECT_REGISTRY_INVALID;
        goto handle_exit;
    }

    /* Compute the total possible objects inside the registry */
    object_entries = ((stop - start) / ((sizeof(struct tracex_object_raw) - sizeof(uint8_t*)) + name_size));
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

tracex_ret_t tracex_object_int_register_resolver_function(struct tracex_object_context *ctx, tracexResolverGetlabel resolverFunc)
{
	tracex_ret_t status;

    /* Sanitize input */
    if (ctx == NULL || resolverFunc == NULL) {
	    status = TRACEX_BAD_INPUT_PTR;
	    goto handle_exit;
    }

    /* Assign the function resolver to the context */
    ctx->resolverFunc = resolverFunc;

    status = TRACEX_SUCCESS;

handle_exit:
	return status;
}

tracex_ret_t tracex_object_int_refresh_resolver_labels(struct tracex_object_context *ctx)
{
    tracex_ret_t status;
    struct tracex_object_entry *entry;

    /* Sanitize input */
    if (ctx == NULL) {
	    status = TRACEX_BAD_INPUT_PTR;
	    goto handle_exit;
    }

    /* Since the resolve function has parsed or deleted labels
     * We need to loop on the already parsed object and update their labels 
     */

     tracex_list_for_each_entry(entry, &ctx->obj_list, node) {
	    resolve_labels(ctx, &entry->obj);
    }

    status = TRACEX_SUCCESS;

handle_exit:
	return status;
}

tracex_ret_t tracex_object_int_parse(struct tracex_object_context *ctx, void *buffer, size_t buff_len, uint64_t *consumed)
{
	tracex_ret_t status;
	size_t bytes_left = buff_len;
	size_t bytes_consumed = 0;

	*consumed = 0;
	status = TRACEX_NEED_MORE;

	while (bytes_left != 0) {
		bytes_consumed = 0;
		status = parse_incrementally(ctx, buffer, bytes_left, &bytes_consumed);

		/* Increment the number of bytes consumed for the caller */
		*consumed += bytes_consumed;

		/* Sanitize for an error */
		if (status != TRACEX_SUCCESS && status != TRACEX_NEED_MORE) {
			goto handle_exit;
		}

		/* We parsed a full object, let's now process it and add it to the list */
		if (status == TRACEX_SUCCESS) {
			/* Increment the number of object parsed for the current session */
			ctx->curr_count++;

			status = process_object(ctx);

			if (status == TRACEX_ALLOC_FAILURE) {
				goto handle_exit;
			}

			if (status == TRACEX_SUCCESS) {
				ctx->tot_count++;
			}

			status = TRACEX_NEED_MORE;

			/* Check if we have parsed the whole object registry*/
			if (ctx->curr_count == ctx->registry_size) {
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
    return status;


}

void tracex_object_destroy_context(struct tracex_object_context *ctx)
{

    /* Destroy the list of parsed objects */
	destroy_object_list(ctx);

    /* In case we were parsing incrementally and that that the parsing got interrupted
     * We need to free the current working entry, otherwise this is a memory leak.
     * As it is not yet added to the current list.
     */

    /*
     * The only way to know if the current entry was in use by the time we are destroying the context
     * Is by either having the current offset being different than 0 and the current_entry different than NUL
     * Or if the current fsm was set to parsing the object name
     */
    if ((ctx->staging_raw_offset != 0 && ctx->tmp_obj != NULL) || ctx->fsm == E_OBJ_PARSE_NAME) {
        destroy_object_entry(&ctx->tmp_obj);
    }
}

tracex_ret_t tracex_object_int_iterator_init(struct tracex_object_context *ctx, TRACEX_object_iterator_t **iterator)
{
    tracex_ret_t status;
    struct tracex_object_entry *entry = NULL;
    uint64_t index = 0;


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
        (*iterator)->objects[index++] = &entry->obj;
    }

    (*iterator)->count = ctx->tot_count;
    (*iterator)->index = 0;

    status = TRACEX_SUCCESS;
    goto handle_exit;
    
handle_error:
    tracex_object_int_iterator_end(iterator);

handle_exit:
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

static void convert_from_raw_to_user(struct tracex_object_context *ctx, struct tracex_object_raw *raw, struct tracex_object *user)
{
	/*TODO: Assign the correct endianess */

	/* The name pointer has already been filled in during the parsing
     * As it was as the main pointer when parsing the name
     */

	user->type = raw->type;
	user->pointer = raw->pointer;
	user->thread_priority = raw->res1;
	user->thread_priority = raw->res2;
	user->params.param1 = raw->param_1;
	user->params.param2 = raw->param_2;

    /* Resolve the labels */
	resolve_labels(ctx, user);
}

static void resolve_labels(struct tracex_object_context *ctx, struct tracex_object *object)
{
    tracex_resolver_labels labels;

    if (ctx->resolverFunc != NULL) {
		labels = ctx->resolverFunc(E_TRACEX_RESOLVER_OBJECT, object->type);

		object->labels.objectTypeName = (labels.objLabels.objectTypeName == NULL) ? default_resolver_invalid_obj_str :
										labels.objLabels.objectTypeName;

        object->labels.param1 = (labels.objLabels.param1 == NULL) ? default_resolver_invalid_params_str :
										labels.objLabels.param1;

        object->labels.param2 = (labels.objLabels.param2 == NULL) ? default_resolver_invalid_params_str :
										labels.objLabels.param2;
	} else {
		object->labels.objectTypeName = default_resolver_invalid_obj_str;
		object->labels.param1 = default_resolver_invalid_params_str;
		object->labels.param2 = default_resolver_invalid_params_str;
	}

}
static tracex_ret_t parse_incrementally(struct tracex_object_context *ctx, void *buffer, size_t buffer_len, size_t *consumed)
{
    tracex_ret_t status;
    void *start_address = NULL;
    size_t bytes_to_copy = 0;
    enum tracex_obj_fsm last_fsm = ctx->fsm; /* Keep track of the previous FSM */


    /* Check if this is a new event. Zero out the staging structure and allocate a new temp object */
    if (ctx->staging_raw_offset == 0 && ctx->fsm == E_OBJ_PARSE_OTHERS)
    {
	    memset(&ctx->staging_raw_obj, 0, sizeof(struct tracex_object_raw));
	    if ((status = alloc_new_object_entry(&ctx->tmp_obj, ctx->name_size)) != TRACEX_SUCCESS) {
		    goto handle_exit;
	    }
        /* Since we just allocated memory for the next object to be parsed, we can take
         * It's just allocated pointer to the object name and have as a working pointer 
         * In the staging object
         */

	    ctx->staging_raw_obj.name = ctx->tmp_obj->obj.name;
    }
    
    /* Are we at the beginning of the parsing ? (The whole struct without the object name) */
    if (ctx->fsm == E_OBJ_PARSE_OTHERS)
    {
        /* Start address is the start of the object struct + the previous offset */
        start_address = ((void*)&ctx->staging_raw_obj) + ctx->staging_raw_offset;

        /* Check if the buffer length added with the previous offset is bigger than the object struct - the size of the object name pointer */
        if (buffer_len + ctx->staging_raw_offset >= (sizeof(struct tracex_object_raw) - sizeof(uint8_t*)))
        {
            /* Copy all the structure fields until the start of the object name pointer in the struct (last field) */
            bytes_to_copy = (sizeof(struct tracex_object_raw) - sizeof(uint8_t*)) - ctx->staging_raw_offset;

            /* Reset the offset for the next loop */
            ctx->staging_raw_offset = 0;

            /* Change the state machine in order to parse the object name */
            ctx->fsm = E_OBJ_PARSE_NAME;

        }
        /* Otherwise it's a partial copy, it's not enough to copy a full struct */
        else
        {
            /* Copy the whole provided buffer */
            bytes_to_copy = buffer_len;

            /* Increment the offset for the next iteration or function call */
            ctx->staging_raw_offset += bytes_to_copy;
        }

    }

    /* We reached the point where we need to copy the object name */
    else if (ctx->fsm == E_OBJ_PARSE_NAME)
    {

        /* Base address is the object name field inside the struct */
        start_address = (void*)((size_t)ctx->staging_raw_obj.name + (size_t)ctx->staging_raw_offset);

        /* Check if the buffer length added with the previous offset is bigger than the object name length */
        if (buffer_len + ctx->staging_raw_offset >= ctx->name_size)
        {
            /* We have enough bytes to copy the full object name */
            bytes_to_copy = ctx->name_size - ctx->staging_raw_offset;

        }
        /* Otherwise it's a partial copy, it's not enough to copy the full struct */
        else
        {
            /* We can't copy the full name at once, so copy only what we can */
            bytes_to_copy = buffer_len;

        }
        /* Increment the offset for the next iteration or function call */
        ctx->staging_raw_offset += bytes_to_copy;
    }

    /* Perform the copy */
    memcpy(start_address, buffer, bytes_to_copy);

    /* Check if we reached the end of the parsing of an object */
    if (last_fsm == E_OBJ_PARSE_NAME && ctx->fsm == E_OBJ_PARSE_NAME && ctx->staging_raw_offset == ctx->name_size)
    {
        /* Reset the FSM in order to parse the object fields again on the next call */
        ctx->fsm = E_OBJ_PARSE_OTHERS;
        
        /* Reset the offset for the next call  */
        ctx->staging_raw_offset = 0;
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
    struct tracex_object_entry *entry = NULL;

    /*  This checks if the available flag is set, which means it shouldn't be added to the list 
    *   Or if the pointer address is set to 0, which means it's an invalid object
    */
    if (ctx->staging_raw_obj.available == 1 || ctx->staging_raw_obj.pointer == 0 ||
        ctx->staging_raw_obj.type == 0) {
        status = TRACEX_OBJECT_INVALID;
        goto handle_exit;

    }

    /* Loop on the current objects */
    tracex_list_for_each_entry(entry, &ctx->obj_list, node)
    {
        /* This might be a destroyed object in the parsing, just ignore it the new object entry then*/
        if (entry->obj.pointer == ctx->staging_raw_obj.pointer)
        {
            status = TRACEX_OBJECT_DUPLICATE;
            goto handle_exit;
        }
        
    }

    /* Convert the raw objet to the user format */
    convert_from_raw_to_user(ctx, &ctx->staging_raw_obj, &ctx->tmp_obj->obj);

    /* Add the object to the list */
    tracex_list_insert(&ctx->tmp_obj->node, &ctx->obj_list);

    status = TRACEX_SUCCESS;

handle_exit:

    if (status == TRACEX_SUCCESS) {
        /* Call the user provided callback */
        if (ctx->on_object_parsed != NULL)
            ctx->on_object_parsed(ctx->cb_data, &ctx->tmp_obj->obj, status);
    } else {
        destroy_object_entry(&ctx->tmp_obj);
    }
    return status;

}

static tracex_ret_t alloc_new_object_entry(struct tracex_object_entry **object_ptr, uint16_t name_length)
{
    tracex_ret_t status;
    struct tracex_object_entry *tmp_entry = NULL;

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
    tmp_entry->obj.name = (uint8_t*)malloc(sizeof(uint8_t) * name_length);
    if (tmp_entry->obj.name == NULL)
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

static void destroy_object_list(struct tracex_object_context *ctx)
{
    struct tracex_object_entry *entry;
    struct tracex_object_entry *next;
    
    
    tracex_list_for_each_entry_safe(entry, next, &ctx->obj_list, node)
    {
        destroy_object_entry(&entry);
    }

}

static void destroy_object_entry(struct tracex_object_entry **object)
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