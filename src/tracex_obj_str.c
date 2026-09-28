#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "tracex_obj_str.h"
#include "cJSON.h"
#include "tracex_list.h"
#include "tracex_utils.h"

#define TRACEX_OBJECT_DEFAULT_JSON_PATH "data/objects.json"

enum label_add_override_setting{
    E_LABEL_ADD_NO_OVERRIDE,
    E_LABEL_ADD_OVERRIDE,
};

struct tracex_obj_json_element {
    cJSON   *value;                
    cJSON   *name;
    cJSON   *param1;
    cJSON   *param2;
};

struct tracex_obj_json {
    struct tracex_obj_json_element          obj_element;
    cJSON                                   *root;
    cJSON                                   *array;
    cJSON                                   *tmp_cjson_elem_ptr;

};

struct tracex_object_labels {
    uint8_t *object_type_str;                   /* The object type string */
    uint32_t object_id;                         /* The object ID as found in the enum tracex_object_type */
    struct tracex_object_params_labels params_str; /* The object strings for the param 1 and param 2 */

    tracex_node node;                           /* The next node */
};


/* In case the list would be empty or a request could not be satisfied,
 * Those will be the default strings when calling one of the functions bellow 
 */
static uint8_t *default_invalid_params = "Not valid"; 
static uint8_t *default_invalid_obj = "Invalid";



/* Actual list of labels for the objects */
struct tracex_list object_labels_list;

static struct tracex_object_labels *alloc_object_labels(size_t count);
static void destroy_object_label(struct tracex_object_labels **obj_label);
static void tracex_object_labels_add(struct tracex_object_labels *new_labels, enum label_add_override_setting override);
static tracex_ret_t convert_cjson_to_obj_label(struct tracex_obj_json_element *cjson_obj, struct tracex_object_labels **new_label);

const uint8_t *tracex_object_type_to_str(uint32_t type)
{

    struct tracex_object_labels *entry;
    uint8_t *string;

    string = default_invalid_obj;

    if (object_labels_list.next != NULL && object_labels_list.prev != NULL) {
        
        tracex_list_for_each_entry(entry, &object_labels_list, node)
        {
            if (entry->object_id == type) {
                string = entry->object_type_str;
            }
        }
    }

    return string;
}

struct tracex_object_params_labels tracex_object_param_to_str(uint32_t type)
{
    struct tracex_object_params_labels params;
    struct tracex_object_labels *entry;

    /* Set the params to invalid in case of an error */
    params.param1_label = default_invalid_params;
    params.param2_label = default_invalid_params;

    if (object_labels_list.next != NULL && object_labels_list.prev != NULL) {

        tracex_list_for_each_entry(entry, &object_labels_list, node)
        {
            if (entry->object_id == type) {
                params.param1_label = entry->params_str.param1_label;
                params.param2_label = entry->params_str.param2_label;
            }
        }
    }
    return params;
}

tracex_ret_t tracex_object_load_labels(uint8_t *json_path)
{
    tracex_ret_t status;
    struct tracex_obj_json json_struct;
    struct tracex_object_labels *labels_ptr;
    uint8_t *actual_path;

    memset(&json_struct, 0, sizeof(struct tracex_obj_json));
    labels_ptr = NULL;

    if (json_path == NULL)
        actual_path = TRACEX_OBJECT_DEFAULT_JSON_PATH;  
    else
        actual_path = json_path;

    status = tracex_utils_load_json_file(actual_path, &json_struct.root);

    if (status != TRACEX_SUCCESS) {
        goto handle_return;
    }

    /* The goal here is to get get each element that is in the array of objectTypesNames
     * Inside the JSON file and 1 struct tracex_object_labels in oder to add that element
     * In the list.
     */

    json_struct.array = cJSON_GetObjectItemCaseSensitive(json_struct.root, "objectTypesNames");

    /* Check if the fields name in the json has been found */
    if (json_struct.array == NULL) {
        status = TRACEX_JSON_PARSING_FAILURE;
        goto handle_return;
    }
    
    /* Get the number of elements and check that it is at least 1 element long  */
    if (cJSON_GetArraySize(json_struct.array) == 0) {
        status = TRACEX_JSON_PARSING_FAILURE;
        goto handle_return;
    }

    /* Iterate on all the element of the array and convert it to a struct tracex_object_labels */
    cJSON_ArrayForEach(json_struct.tmp_cjson_elem_ptr, json_struct.array) {

        /* Get the value field */
        json_struct.obj_element.value = cJSON_GetObjectItemCaseSensitive(json_struct.tmp_cjson_elem_ptr, "value");        
        /* Get the name string value */
        json_struct.obj_element.name = cJSON_GetObjectItemCaseSensitive(json_struct.tmp_cjson_elem_ptr, "name");
        /* Get the different params */
        json_struct.obj_element.param1 = cJSON_GetObjectItemCaseSensitive(json_struct.tmp_cjson_elem_ptr, "param1");
        json_struct.obj_element.param2 = cJSON_GetObjectItemCaseSensitive(json_struct.tmp_cjson_elem_ptr, "param2");

        /* Convert the cJSON object to a struct tracex_object_labels and add it */
        if (convert_cjson_to_obj_label(&json_struct.obj_element, &labels_ptr) == TRACEX_SUCCESS) {

            /* We finally add the converted tracex_object_labels to the global list */
            tracex_object_labels_add(labels_ptr, E_LABEL_ADD_NO_OVERRIDE);
        }
    }


handle_return:

    /*  Free the parsed json objects as we don't need them anymore
    *   They have been converted to struct tracex_object_labels and are part of a list
    */

    if (json_struct.root != NULL) {
        cJSON_Delete(json_struct.root);
    }
    return status;
}

void tracex_object_destroy_labels(void)
{
    struct tracex_object_labels *entry;
    struct tracex_object_labels *next;

    if (object_labels_list.next != NULL && object_labels_list.prev != NULL) {

        tracex_list_for_each_entry_safe(entry, next, &object_labels_list,node)
        {
            destroy_object_label(&entry);
        }

    }
}


static void tracex_object_labels_add(struct tracex_object_labels *new_labels, enum label_add_override_setting override)
{
    struct tracex_object_labels *next;
    uint8_t found;

    found = 0;

    /* In case we are adding the very first element of the list, we need to initialize the list */
    if (object_labels_list.prev == NULL || object_labels_list.next == NULL) {
        tracex_list_init(&object_labels_list);
    }

    /* If override is not allowed, add the labels to the list */
    if (override == E_LABEL_ADD_NO_OVERRIDE)
        goto early_add_list;
    else {

        /* Otherwise, we need to check if a label of the same type exists and potentially override it */
        tracex_list_for_each_entry(next, &object_labels_list, node)
        {
            /* The object ID is already in the list. Let's replace it */
            if (next->object_id == new_labels->object_id) {
                
                new_labels->node.next = next->node.next;
                new_labels->node.prev = next->node.prev;
                next->node.prev->next = &new_labels->node;
                next->node.next->prev = &new_labels->node;

                /*  Since the destroy function bellow moves the pointers inside the list
                 *  We need to avoid it doing so. One single way is to set it's pointer to NULL
                 *  Otherwise it would remove the new_labels previously added from the list
                */ 
                next->node.next = NULL;
                next->node.prev = NULL;
                /* Destroy the object from the list */
                  destroy_object_label(&next);
                /* Set the flag to avoid adding it again */
                found = 1;
                break;

            }

        }

    }
    
early_add_list:
    if (found == 0) {
        /* Add the actual element in the list */
        tracex_list_insert(&new_labels->node, &object_labels_list);
    }

early_return:
    return;
}


static void destroy_object_label(struct tracex_object_labels **obj_label)
{
    if (obj_label != NULL) {

        if (*obj_label != NULL) {

            /* Destroy the object type string */
            if ((*obj_label)->object_type_str != NULL) {
                free((*obj_label)->object_type_str);
                (*obj_label)->object_type_str = NULL;
            }

            /* Destroy the object param 1 string */
            if ((*obj_label)->params_str.param1_label != NULL) {
                free((void*)(*obj_label)->params_str.param1_label);
                (*obj_label)->params_str.param1_label = NULL;
            }

            /* Destroy the object param 2 string */
            if ((*obj_label)->params_str.param2_label != NULL) {
                free((void*)(*obj_label)->params_str.param2_label);
                (*obj_label)->params_str.param2_label = NULL;
            }

            /* Check if the object is part of a list and remote it */
            if ((*obj_label)->node.next != NULL && (*obj_label)->node.prev != NULL) {
                tracex_list_delete(&(*obj_label)->node);
                (*obj_label)->node.next = NULL;
                (*obj_label)->node.prev = NULL;
            }

            free(*obj_label);
            *obj_label = NULL;
        }
    }
}

static struct tracex_object_labels *alloc_object_labels(size_t count)
{
    struct tracex_object_labels *tmp;

    tmp = NULL;

    if (count == 0)
        goto early_return;
    
    tmp = (struct tracex_object_labels*)malloc(sizeof(struct tracex_object_labels) * count);

    if (tmp == NULL)
        goto early_return;
    

early_return:
    return tmp;

}


static tracex_ret_t convert_cjson_to_obj_label(struct tracex_obj_json_element *cjson_obj, struct tracex_object_labels **new_label)
{
    struct tracex_object_labels *tmp_label;
    uint8_t *tmp_buffer;
    tracex_ret_t status;


    /* Check for any missing field */
    if (cjson_obj->name == NULL || cjson_obj->param1 == NULL || cjson_obj->param2 == NULL)
    {
        status = TRACEX_BAD_INPUT_PTR;
        goto handle_return;
    }

    /* Check for a NULL string value */

    if (cjson_obj->name->valuestring == NULL ||
        cjson_obj->param1->valuestring == NULL ||
        cjson_obj->param2->valuestring == NULL) {
            status = TRACEX_JSON_PARSING_FAILURE;
            goto handle_return;
        }
    
    /* Alloc a new object label */
    tmp_label = alloc_object_labels(1);

    /* Check if successfull */
    if (tmp_label == NULL) {
        status = TRACEX_ALLOC_FAILURE;
        goto handle_return;
    }

    /* Erase the newly allocated object */
    memset(tmp_label, 0, sizeof(struct tracex_object_labels));

    /* Duplicate the type label string  */
    tmp_buffer = (uint8_t*)malloc((sizeof(uint8_t) * strlen(cjson_obj->name->valuestring)) + 1);

    if (tmp_buffer == NULL) {
        status = TRACEX_ALLOC_FAILURE;
        goto handle_error;
    }

    memcpy(tmp_buffer, cjson_obj->name->valuestring, strlen(cjson_obj->name->valuestring) + 1);
    tmp_label->object_type_str = tmp_buffer;
    
    /* Duplicate the param1 label string */
    tmp_buffer = (uint8_t*)malloc((sizeof(uint8_t) * strlen(cjson_obj->param1->valuestring)) + 1);
    
    if (tmp_buffer == NULL) {
        status = TRACEX_ALLOC_FAILURE;
        goto handle_error;
    }
    memcpy(tmp_buffer, cjson_obj->param1->valuestring, strlen(cjson_obj->param1->valuestring) + 1);
    tmp_label->params_str.param1_label = tmp_buffer;

    /* Duplicate the param2 label string */
    tmp_buffer = (uint8_t*)malloc((sizeof(uint8_t) * strlen(cjson_obj->param2->valuestring)) + 1);
    
    if (tmp_buffer == NULL) {
        status = TRACEX_ALLOC_FAILURE;
        goto handle_error;
    }
    memcpy(tmp_buffer, cjson_obj->param2->valuestring, strlen(cjson_obj->param2->valuestring) + 1);
    tmp_label->params_str.param2_label = tmp_buffer;


    /* Copy over the object ID */
    tmp_label->object_id = cjson_obj->value->valueint;

    status = TRACEX_SUCCESS;

    goto handle_return;

handle_error:
    /* Destroy the newly allocated object label and all it's pointers */
    destroy_object_label(&tmp_label);

handle_return:
    *new_label = tmp_label;
    return status;
}