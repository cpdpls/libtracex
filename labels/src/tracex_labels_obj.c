#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "uthash.h"
#include "tracex/tracex_labels.h"
#include "cJSON.h"
#include "tracex_labels_utils.h"

#define TRACEX_OBJECT_DEFAULT_JSON_PATH "data/objects.json"

/* In case a param label for a given object type does not exist,
 * This string will be assigned to the object
 */
static uint8_t *default_invalid_params = "Not valid"; 

/* In case an object type label for a given type does not exist,
 * This string will be assigned to the object
 */
static uint8_t *default_invalid_obj = "Invalid";


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

struct tracex_labels_object_labels {
    uint8_t *object_type_str;                   /* The object type string */
    uint32_t object_id;                         /* The object ID as found in the enum tracex_object_type */
    struct tracex_labels_object_params_labels params_str; /* The object strings for the param 1 and param 2 */

    UT_hash_handle hh;                           /* The next node */
};


/* Global variable that holds all the parsed labels from a json file using a hash table */
static struct tracex_labels_object_labels *object_labels_hash = NULL;

static struct tracex_labels_object_labels *alloc_object_labels(size_t count);
static void destroy_object_label(struct tracex_labels_object_labels **obj_label);
static void tracex_labels_object_labels_add(struct tracex_labels_object_labels *new_labels, enum label_add_override_setting override);
static tracex_labels_ret_t convert_cjson_to_obj_label(struct tracex_obj_json_element *cjson_obj, struct tracex_labels_object_labels **new_label);

const uint8_t *tracex_object_type_to_str(uint32_t type)
{

    struct tracex_labels_object_labels *entry = NULL;
    uint8_t *string = NULL;

    /* First check if the provided type corresponds to a set of previously parsed object type labels from the json file */
	HASH_FIND_INT(object_labels_hash, &type, entry);

    /* If the type is part of the hashed list, we can return it's type label string */
	if (entry)
		string = entry->object_type_str;
	
	/* Othwersise, the ID is not linked to any existing label parsed from the json. Assign it the default string */
	else
		string = default_invalid_obj;

    return string;
}

struct tracex_labels_object_params_labels tracex_labels_object_param_to_str(uint32_t type)
{
    struct tracex_labels_object_params_labels params;
    struct tracex_labels_object_labels *entry = NULL;

    /* First check if the provided ID corresponds to a set of previously parsed string labels from the json file */

	HASH_FIND_INT(object_labels_hash, &type, entry);

	if (entry) {
		params.param1_label = entry->params_str.param1_label;
		params.param1_label = entry->params_str.param2_label;

	} else {
		/* Set the params to invalid in case of an error */
		params.param1_label = default_invalid_params;
		params.param1_label = default_invalid_params;
	}
	return params;
}

tracex_labels_ret_t tracex_object_load_labels(uint8_t *json_path)
{
    tracex_labels_ret_t status;
    struct tracex_obj_json json_struct;
    struct tracex_labels_object_labels *labels_ptr = NULL;
    uint8_t *actual_path = NULL;

    memset(&json_struct, 0, sizeof(struct tracex_obj_json));

    if (json_path == NULL)
        actual_path = TRACEX_OBJECT_DEFAULT_JSON_PATH;  
    else
        actual_path = json_path;

    status = tracex_utils_load_json_file(actual_path, &json_struct.root);

    if (status != TRACEX_LABELS_SUCCESS) {
        goto handle_return;
    }

    /* The goal here is to get get each element that is in the array of objectTypesNames
     * Inside the JSON file and 1 struct tracex_labels_object_labels in oder to add that element
     * In the list.
     */

    json_struct.array = cJSON_GetObjectItemCaseSensitive(json_struct.root, "objectTypesNames");

    /* Check if the fields name in the json has been found */
    if (json_struct.array == NULL) {
        status = TRACEX_LABELS_JSON_PARSING_FAILURE;
        goto handle_return;
    }
    
    /* Get the number of elements and check that it is at least 1 element long  */
    if (cJSON_GetArraySize(json_struct.array) == 0) {
        status = TRACEX_LABELS_JSON_PARSING_FAILURE;
        goto handle_return;
    }

    /* Iterate on all the element of the array and convert it to a struct tracex_labels_object_labels */
    cJSON_ArrayForEach(json_struct.tmp_cjson_elem_ptr, json_struct.array) {

        /* Get the value field */
        json_struct.obj_element.value = cJSON_GetObjectItemCaseSensitive(json_struct.tmp_cjson_elem_ptr, "value");        
        /* Get the name string value */
        json_struct.obj_element.name = cJSON_GetObjectItemCaseSensitive(json_struct.tmp_cjson_elem_ptr, "name");
        /* Get the different params */
        json_struct.obj_element.param1 = cJSON_GetObjectItemCaseSensitive(json_struct.tmp_cjson_elem_ptr, "param1");
        json_struct.obj_element.param2 = cJSON_GetObjectItemCaseSensitive(json_struct.tmp_cjson_elem_ptr, "param2");

        /* Convert the cJSON object to a struct tracex_labels_object_labels and add it */
        if (convert_cjson_to_obj_label(&json_struct.obj_element, &labels_ptr) == TRACEX_LABELS_SUCCESS) {

            /* We finally add the converted tracex_labels_object_labels to the global list */
            tracex_labels_object_labels_add(labels_ptr, E_LABEL_ADD_NO_OVERRIDE);
        }
    }


handle_return:

    /*  Free the parsed json objects as we don't need them anymore
    *   They have been converted to struct tracex_labels_object_labels and are part of a list
    */

    if (json_struct.root != NULL) {
        cJSON_Delete(json_struct.root);
    }
    return status;
}

void tracex_object_destroy_labels(void)
{
    struct tracex_labels_object_labels *entry = NULL;
    struct tracex_labels_object_labels *tmp = NULL;

    /* Walk the hash table and free everything */
	HASH_ITER(hh, object_labels_hash, entry, tmp)
	{
		destroy_object_label(&entry);
	}
}


static void tracex_labels_object_labels_add(struct tracex_labels_object_labels *new_labels, enum label_add_override_setting override)
{
    struct tracex_labels_object_labels *existing = NULL;
    uint8_t found = 0;

    HASH_FIND_INT(object_labels_hash, &new_labels->object_id, existing);

    /* If the event is found and override is allowed, delete the existing*/
	if (existing) {
		if (override == E_LABEL_ADD_OVERRIDE) {
			destroy_object_label(&new_labels);
		} else
			goto early_return;
	}

early_add_list:

	/* Add the actual element in the list */
	HASH_ADD_INT(object_labels_hash, object_id, new_labels);

early_return:
	return;
}


static void destroy_object_label(struct tracex_labels_object_labels **obj_label)
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

            HASH_DEL(object_labels_hash, *obj_label);

            free(*obj_label);
            *obj_label = NULL;
        }
    }
}

static struct tracex_labels_object_labels *alloc_object_labels(size_t count)
{
    struct tracex_labels_object_labels *tmp = NULL;

    if (count == 0)
        goto early_return;
    
    tmp = (struct tracex_labels_object_labels*)malloc(sizeof(struct tracex_labels_object_labels) * count);

    if (tmp == NULL)
        goto early_return;
    

early_return:
    return tmp;

}


static tracex_labels_ret_t convert_cjson_to_obj_label(struct tracex_obj_json_element *cjson_obj, struct tracex_labels_object_labels **new_label)
{
    tracex_labels_ret_t status;
    struct tracex_labels_object_labels *tmp_label = NULL;
    uint8_t *tmp_buffer = NULL;


    /* Check for any missing field */
    if (cjson_obj->name == NULL || cjson_obj->param1 == NULL || cjson_obj->param2 == NULL)
    {
        status = TRACEX_LABELS_BAD_INPUT_PTR;
        goto handle_return;
    }

    /* Check for a NULL string value */

    if (cjson_obj->name->valuestring == NULL ||
        cjson_obj->param1->valuestring == NULL ||
        cjson_obj->param2->valuestring == NULL) {
            status = TRACEX_LABELS_JSON_PARSING_FAILURE;
            goto handle_return;
        }
    
    /* Alloc a new object label */
    tmp_label = alloc_object_labels(1);

    /* Check if successfull */
    if (tmp_label == NULL) {
        status = TRACEX_LABELS_ALLOC_FAILURE;
        goto handle_return;
    }

    /* Erase the newly allocated object */
    memset(tmp_label, 0, sizeof(struct tracex_labels_object_labels));

    /* Duplicate the type label string  */
    tmp_buffer = (uint8_t*)malloc((sizeof(uint8_t) * strlen(cjson_obj->name->valuestring)) + 1);

    if (tmp_buffer == NULL) {
        status = TRACEX_LABELS_ALLOC_FAILURE;
        goto handle_error;
    }

    memcpy(tmp_buffer, cjson_obj->name->valuestring, strlen(cjson_obj->name->valuestring) + 1);
    tmp_label->object_type_str = tmp_buffer;
    
    /* Duplicate the param1 label string */
    tmp_buffer = (uint8_t*)malloc((sizeof(uint8_t) * strlen(cjson_obj->param1->valuestring)) + 1);
    
    if (tmp_buffer == NULL) {
        status = TRACEX_LABELS_ALLOC_FAILURE;
        goto handle_error;
    }
    memcpy(tmp_buffer, cjson_obj->param1->valuestring, strlen(cjson_obj->param1->valuestring) + 1);
    tmp_label->params_str.param1_label = tmp_buffer;

    /* Duplicate the param2 label string */
    tmp_buffer = (uint8_t*)malloc((sizeof(uint8_t) * strlen(cjson_obj->param2->valuestring)) + 1);
    
    if (tmp_buffer == NULL) {
        status = TRACEX_LABELS_ALLOC_FAILURE;
        goto handle_error;
    }
    memcpy(tmp_buffer, cjson_obj->param2->valuestring, strlen(cjson_obj->param2->valuestring) + 1);
    tmp_label->params_str.param2_label = tmp_buffer;


    /* Copy over the object ID */
    tmp_label->object_id = cjson_obj->value->valueint;

    status = TRACEX_LABELS_SUCCESS;

    goto handle_return;

handle_error:
    /* Destroy the newly allocated object label and all it's pointers */
    destroy_object_label(&tmp_label);

handle_return:
    *new_label = tmp_label;
    return status;
}