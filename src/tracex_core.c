#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>

#include "cJSON.h"
#include "tracex_errno.h"
#include "tracex_core.h"
#include "tracex_event_int.h"
#include "tracex_obj_int.h"
#include "tracex_utils.h"
#include "tracex_header.h"
#include "tracex_event.h"
#include "tracex_object.h"
#include "tracex_obj_str.h"

struct tracex_core_object_labels_json {
    cJSON   *array;
    cJSON   *value;                
    cJSON   *name;
    cJSON   *param1;
    cJSON   *param2;
};

struct tracex_core_object_json {
    struct tracex_core_object_labels_json   obj_types;
    cJSON                                   *root;
    cJSON                                   *tmp_item;  

};

uint8_t tracex_init_done;       /* Flag set to detect wether tracex has been init */


static void core_on_header_parsed_callback(void *cb_data, struct tracex_header *header, tracex_ret_t status);
static void core_on_object_parsed_callback(void *cb_data, struct tracex_object *object, tracex_ret_t status);
static void core_on_event_parsed_callback(void *cb_data, struct tracex_event *event, tracex_ret_t status);
static tracex_ret_t load_json_objects_labels(void);
static tracex_ret_t parse_json_objects_labels(cJSON **root);

tracex_ret_t tracex_init(void)
{
    tracex_ret_t status;

    /* First try to detect the endianess of the host system */
    tracex_utils_detect_indianess();

    /*  Let's first try to parse the object json in order to get the names
     *  for the types of object names and the parameters name
    */

    status = load_json_objects_labels();

    if (status == TRACEX_SUCCESS) {
        tracex_init_done = 1;
    }

    return status;

}

void tracex_deinit(void)
{
    tracex_object_destroy_labels();
}

/*TODO: Since we are using a global variable for the list of created handlers, let's also use a mutex for when creating new onces */
tracex_ret_t tracex_create_new_handler(tracex_handler_t **new_handler)
{
    struct tracex_handler *handler;
    tracex_ret_t status;


    /* Check if tracex has been initialized correctly */
    if (!tracex_init_done)
    {
        status = TRACEX_NOT_INIT;
        goto handle_exit;
    }

    if (new_handler == NULL)
    {
        status = TRACEX_BAD_INPUT_PTR;
        goto handle_exit;
    }
    handler = (struct tracex_handler*)malloc(sizeof(struct tracex_handler));

    if (handler == NULL)
    {
        status = TRACEX_ALLOC_FAILURE;
        goto handle_exit;
    }

    /* Faster to memset the structure to 0 */
    memset(handler, 0, sizeof(struct tracex_handler));

    /* Init the different contexts parts */
    if ((status = tracex_header_int_init(&handler->hdr_ctx)) != TRACEX_SUCCESS)
    {
        status = TRACEX_INIT_FAILURE;
        goto handle_error;
    }

    if ((status = tracex_object_int_init(&handler->objs_ctx)) != TRACEX_SUCCESS)
    {
        status = TRACEX_INIT_FAILURE;
        goto handle_error;
    }

    if ((status = tracex_event_int_init(&handler->event_ctx)) != TRACEX_SUCCESS)
    {
        status = TRACEX_INIT_FAILURE;
        goto handle_error;
    }

    *new_handler = handler;
    status = TRACEX_SUCCESS;
    goto handle_exit;

handle_error:
    free(handler);
    handler = NULL;

handle_exit:

    return status;
}

tracex_ret_t tracex_register_callbacks(tracex_handler_t *handler, struct tracex_callbacks *callbacks)
{
    if (callbacks == NULL || handler == NULL)
    {
        return TRACEX_BAD_INPUT_PTR;
    }

    /* Assign the user provided callbacks internally to the handler regardless of their pointer value */
    handler->user_callbacks.on_header_parsed = callbacks->on_header_parsed;
    handler->user_callbacks.on_object_parsed = callbacks->on_object_parsed;
    handler->user_callbacks.on_event_parsed = callbacks->on_event_parsed;

    /* Check each individual callback to be different that NULL and assign the internal callback, otherwise NULL */
    handler->hdr_ctx.on_header_parsed = callbacks->on_header_parsed != NULL ? core_on_header_parsed_callback : NULL;
    handler->objs_ctx.on_object_parsed = callbacks->on_object_parsed != NULL ? core_on_object_parsed_callback : NULL;
    handler->event_ctx.on_event_parsed = callbacks->on_event_parsed != NULL ? core_on_event_parsed_callback : NULL;

    /* Assign the handler as a callback data */
    handler->hdr_ctx.cb_data = (void*)handler;
    handler->objs_ctx.cb_data = (void*)handler;
    handler->event_ctx.cb_data = (void*)handler;

    return TRACEX_SUCCESS;

}

void tracex_destroy_handler(tracex_handler_t **handler)
{
    if (handler != NULL)
    {
        if (*handler != NULL)
        {
            
            tracex_object_int_destroy_list(&(*handler)->objs_ctx);
            tracex_event_int_destroy_list(&(*handler)->event_ctx);
            memset((*handler), 0, sizeof(struct tracex_handler));
            free(*handler);
            *handler = NULL;
            
        }
    }

}

tracex_ret_t tracex_parse(struct tracex_handler *handler, void *buffer, size_t buffer_length)
{
    tracex_ret_t status;
    uint64_t consumed;


    struct tracex_header_context *hdr_ctx;
    consumed = 0;

    status = TRACEX_NEED_MORE;

    if (handler == NULL || buffer == NULL)
    {
        status = TRACEX_BAD_INPUT_PTR;
        goto handle_exit;
    }

    if (buffer_length == 0)
    {
        status = TRACEX_NULL_LENGTH;
        goto handle_exit;
    }


    hdr_ctx = &handler->hdr_ctx;

    if (hdr_ctx->header_parsed && !hdr_ctx->header_valid)
    {
        status = TRACEX_HEADER_NOT_VALID;
        goto handle_exit;
    }

    /* Are we in the beginning of the parsing, AKA parsing the header ? */
    if (handler->state == E_HEADER_PHASE)
    {
        status = tracex_header_int_parse(hdr_ctx, buffer, buffer_length, &consumed);

        if (status == TRACEX_SUCCESS)
        {
            /* Increment the total bytes processed from here */
            handler->raw_bytes_count += consumed;

            /* Assign the needed missing values for the next phases */
            handler->objs_ctx.name_size = handler->hdr_ctx.header.obj_registry_name_size;
            handler->objs_ctx.registry_size = handler->hdr_ctx.obj_registry_size;
            handler->event_ctx.registry_size = handler->hdr_ctx.event_registry_size;

            /* Go to the next phase, which is parsing the objects */
            handler->state = E_OBJECT_PHASE;

            /* If the remaining for the next phase is 0, ask for more */
            if (buffer_length - consumed == 0)
            {
                status = TRACEX_NEED_MORE;
                goto handle_exit;
            }
            /*  Enough bytes for the next phase, increment the buffer pointer 
            *   And substract the buffer length with the just consumed total
            */
            else
            {
                buffer += consumed;
                buffer_length -= consumed;
            }
        }
    }

    /* We are now trying to parse the objects */
    if (handler->state == E_OBJECT_PHASE)
    {
        status = tracex_object_int_parse(&handler->objs_ctx, buffer, buffer_length, &consumed);

        if (status == TRACEX_SUCCESS)
        {
            handler->state = E_EVENT_PHASE;
        }
        
        handler->raw_bytes_count += consumed;

        /* If the remaining for the next phase is 0, ask for more */
        if (buffer_length - consumed == 0)
        {
            status = TRACEX_NEED_MORE;
            goto handle_exit;
        }
        /*  Enough bytes for the next phase, increment the buffer pointer 
        *   And substract the buffer length with the just consumed total
        */
        else
        {
            buffer += consumed;
            buffer_length -= consumed;
        }


    }
    
    if (handler->state == E_EVENT_PHASE)
    {
        status = tracex_event_int_parse(&handler->event_ctx, buffer, buffer_length, &consumed);

        if (status == TRACEX_SUCCESS)
        {
            handler->state = E_OBJECT_PHASE;
        }

        handler->raw_bytes_count += consumed;
        buffer += consumed;
        buffer_length -= consumed;
    }

handle_exit:
    return status;
}

tracex_ret_t tracex_object_iterator_init(tracex_handler *handler, TRACEX_object_iterator_t **iterator)
{

    if (handler == NULL || iterator == NULL)
    {
        return TRACEX_BAD_INPUT_PTR;
    }

    return tracex_object_int_iterator_init(&handler->objs_ctx, iterator);

}

tracex_ret_t tracex_object_iterator_next(TRACEX_object_iterator_t *iterator, const struct tracex_object **object)
{
    if (iterator == NULL || object == NULL)
    {
        return TRACEX_BAD_INPUT_PTR;
    }

    return tracex_object_int_iterator_next(iterator, object);
}

void tracex_object_iterator_end(TRACEX_object_iterator_t **iterator)
{
    if (iterator != NULL)
    {
        tracex_object_int_iterator_end(iterator);
    }

}

static void core_on_header_parsed_callback(void *cb_data, struct tracex_header *header, tracex_ret_t status)
{
    struct tracex_handler *tmp_handler;

    tmp_handler = TO_HANDLER(cb_data);

    /* Call the user callback if not NULL */
    if (tmp_handler->user_callbacks.on_header_parsed != NULL)
        tmp_handler->user_callbacks.on_header_parsed(tmp_handler, header, status);

}
static void core_on_object_parsed_callback(void *cb_data, struct tracex_object *object, tracex_ret_t status)
{
    struct tracex_handler *tmp_handler;

    tmp_handler = TO_HANDLER(cb_data);

    /* Call the user callback if not NULL */
    if (tmp_handler->user_callbacks.on_object_parsed != NULL)
        tmp_handler->user_callbacks.on_object_parsed(tmp_handler, object, status);

}
static void core_on_event_parsed_callback(void *cb_data, struct tracex_event *event, tracex_ret_t status)
{
    struct tracex_handler *tmp_handler;

    tmp_handler = TO_HANDLER(cb_data);

    /* Call the user callback if not NULL */
    if (tmp_handler->user_callbacks.on_event_parsed != NULL)
        tmp_handler->user_callbacks.on_event_parsed(tmp_handler, event, status);
}

static tracex_ret_t load_json_objects_labels(void)
{
    tracex_ret_t status;
    struct tracex_core_object_json jsons;
    struct tracex_object_labels obj_labels;
    

    status = parse_json_objects_labels(&jsons.root);

    if (status != TRACEX_SUCCESS) {
        goto handle_return;
    }


    /* Check if the key exists */
    jsons.obj_types.array = cJSON_GetObjectItemCaseSensitive(jsons.root, "objectTypesNames");
    if (jsons.obj_types.array == NULL) {
        status = TRACEX_JSON_FAILURE;
        goto handle_return;
    }

    /* Now, get the size of the array inside the json parsed file */
    /* And check for an empty array and return an error */
    if (cJSON_GetArraySize(jsons.obj_types.array) == 0) {
        status = TRACEX_JSON_FAILURE;
        goto handle_return;

    }

    /* Iterate on all the element of the arary */
    cJSON_ArrayForEach(jsons.tmp_item, jsons.obj_types.array) {

        /* Get the value field */
        jsons.obj_types.value = cJSON_GetObjectItemCaseSensitive(jsons.tmp_item, "value");
        obj_labels.object_id = jsons.obj_types.value->valueint;
        
        /* Get the name string value */
        jsons.obj_types.name = cJSON_GetObjectItemCaseSensitive(jsons.tmp_item, "name");
        obj_labels.object_type_str = cJSON_GetStringValue(jsons.obj_types.name);

        /* Get the different params */
        jsons.obj_types.param1 = cJSON_GetObjectItemCaseSensitive(jsons.tmp_item, "param1");
        jsons.obj_types.param2 = cJSON_GetObjectItemCaseSensitive(jsons.tmp_item, "param2");

        obj_labels.params_str.param1_label = cJSON_GetStringValue(jsons.obj_types.param1);
        obj_labels.params_str.param2_label = cJSON_GetStringValue(jsons.obj_types.param2);


        tracex_object_labels_add(&obj_labels);
    }

    status = TRACEX_SUCCESS;
    goto handle_return;
    

handle_return:

    if (jsons.root != NULL) {
        cJSON_Delete(jsons.root);
    }
    return status;
    
}

static tracex_ret_t parse_json_objects_labels(cJSON **root)
{
    tracex_ret_t status;
    FILE *file_ptr;
    size_t file_size;
    char *tmp_buff;

    /* Try to open the actual file */
    file_ptr = fopen("data/objects.json", "rb");

    if (file_ptr == NULL) {
        status = TRACEX_JSON_FAILURE;
        goto handle_return;
    }

    /* Get the file size */
    fseek(file_ptr, 0, SEEK_END);
    file_size = ftell(file_ptr);
    fseek(file_ptr, 0, SEEK_SET);

    /* Check if not empty */
    if (file_size == 0) {
        status = TRACEX_JSON_FAILURE;
        goto handle_return;
    }


    /* Allocate the whole buffer and load the file inside that */
    tmp_buff = (char*)malloc(sizeof(char) *file_size);

    if (tmp_buff == NULL) {
        status = TRACEX_ALLOC_FAILURE;
        goto handle_return;
    }

    if (fread(tmp_buff, 1, file_size, file_ptr) != file_size) {
        status = TRACEX_JSON_FAILURE;
        goto handle_return;
    }

    /* Parse the actual json object file */
    *root = cJSON_ParseWithLength(tmp_buff, file_size);

    if (*root == NULL) {
        status = TRACEX_JSON_FAILURE;
        goto handle_return;
    }

    status = TRACEX_SUCCESS;

handle_return:

    if (tmp_buff != NULL) {
        free(tmp_buff);
        tmp_buff = NULL;
    }

    if (file_ptr != NULL) {
        fclose(file_ptr);
        file_ptr = NULL;
    }

    return status;
}