#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "uthash.h"
#include "tracex_resolver/tracex_labels.h"
#include "cJSON.h"
#include "tracex_labels_utils.h"

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

struct tracex_labels_object_labels {
    uint32_t object_id;                         /* The object ID as found in the enum tracex_object_type */
    struct tracex_object_labels labels; /* The object strings for the param 1 and param 2 */

    UT_hash_handle hh;                           /* The next node */
};


/* Global variable that holds all the parsed labels from a json file using a hash table */
static struct tracex_labels_object_labels *object_labels_hash = NULL;

static struct tracex_labels_object_labels *alloc_object_labels(size_t count);
static void destroy_object_label(struct tracex_labels_object_labels **obj_label);
static void tracex_labels_object_labels_add(struct tracex_labels_object_labels *new_labels, enum label_add_override_setting override);
static int convert_cjson_to_obj_label(struct tracex_obj_json_element *cjson_obj, struct tracex_labels_object_labels **new_label);

struct tracex_object_labels tracex_resolver_get_object_labels(uint32_t type)
{
    struct tracex_labels_object_labels *entry = NULL;
    struct tracex_object_labels labels;

    /* First check if the provided type corresponds to a set of previously parsed object labels from the json file */
    HASH_FIND_INT(object_labels_hash, &type, entry);

    /* If the type is part of the hashed list, we can return the object labels  */
	if (entry)
		labels = entry->labels;

    /* Othwersise, the type is not linked to any existing label parsed from the json. Assign it the default string */
    else
        memset(&labels, 0, sizeof(struct tracex_object_labels));

    return labels;
}

int tracex_resolver_object_load_labels(const char *json_path)
{
    int status;
    struct tracex_obj_json json_struct;
    struct tracex_labels_object_labels *labels_ptr = NULL;
    const char *actual_path = NULL;

    memset(&json_struct, 0, sizeof(struct tracex_obj_json));

    if (json_path == NULL)
        actual_path = TRACEX_OBJECT_DEFAULT_JSON_PATH;  
    else
        actual_path = json_path;

    status = tracex_resolver_utils_load_json_file(actual_path, &json_struct.root);

    if (status != 0) {
        goto handle_return;
    }

    /* The goal here is to get get each element that is in the array of objectTypesNames
     * Inside the JSON file and 1 struct tracex_labels_object_labels in oder to add that element
     * In the list.
     */

    json_struct.array = cJSON_GetObjectItemCaseSensitive(json_struct.root, "objectTypesNames");

    /* Check if the fields name in the json has been found */
    if (json_struct.array == NULL) {
        status = -1;
        goto handle_return;
    }
    
    /* Get the number of elements and check that it is at least 1 element long  */
    if (cJSON_GetArraySize(json_struct.array) == 0) {
        status = -1;
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
        if (convert_cjson_to_obj_label(&json_struct.obj_element, &labels_ptr) == 0) {

            /* We finally add the converted tracex_labels_object_labels to the global list */
            tracex_labels_object_labels_add(labels_ptr, E_LABEL_ADD_NO_OVERRIDE);
        }
    }

    status = 0;

handle_return:

    /*  Free the parsed json objects as we don't need them anymore
    *   They have been converted to struct tracex_labels_object_labels and are part of a list
    */

    if (json_struct.root != NULL) {
        cJSON_Delete(json_struct.root);
    }
    return status;
}

void tracex_resolver_object_destroy_labels(void)
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
            if ((*obj_label)->labels.objectTypeName != NULL) {
                free((void *)(*obj_label)->labels.objectTypeName);
                (*obj_label)->labels.objectTypeName = NULL;
            }

            /* Destroy the object param 1 string */
            if ((*obj_label)->labels.param1 != NULL) {
                free((void*)(*obj_label)->labels.param1);
                (*obj_label)->labels.param1 = NULL;
            }

            /* Destroy the object param 2 string */
            if ((*obj_label)->labels.param2 != NULL) {
                free((void*)(*obj_label)->labels.param2);
                (*obj_label)->labels.param2 = NULL;
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


static int convert_cjson_to_obj_label(struct tracex_obj_json_element *cjson_obj, struct tracex_labels_object_labels **new_label)
{
    int status;
    struct tracex_labels_object_labels *tmp_label = NULL;
    uint8_t *tmp_buffer = NULL;


    /* Check for any missing field */
    if (cjson_obj->name == NULL || cjson_obj->param1 == NULL || cjson_obj->param2 == NULL)
    {
        status = -1;
        goto handle_return;
    }

    /* Check for a NULL string value */

    if (cjson_obj->name->valuestring == NULL ||
        cjson_obj->param1->valuestring == NULL ||
        cjson_obj->param2->valuestring == NULL) {
            status = -1;
            goto handle_return;
        }
    
    /* Alloc a new object label */
    tmp_label = alloc_object_labels(1);

    /* Check if successfull */
    if (tmp_label == NULL) {
        status = -1;
        goto handle_return;
    }

    /* Erase the newly allocated object */
    memset(tmp_label, 0, sizeof(struct tracex_labels_object_labels));

    /* Duplicate the type label string  */
    tmp_buffer = (uint8_t*)malloc((sizeof(uint8_t) * strlen(cjson_obj->name->valuestring)) + 1);

    if (tmp_buffer == NULL) {
        status = -1;
        goto handle_error;
    }

    memcpy(tmp_buffer, cjson_obj->name->valuestring, strlen(cjson_obj->name->valuestring) + 1);
    tmp_label->labels.objectTypeName = tmp_buffer;
    
    /* Duplicate the param1 label string */
    tmp_buffer = (uint8_t*)malloc((sizeof(uint8_t) * strlen(cjson_obj->param1->valuestring)) + 1);
    
    if (tmp_buffer == NULL) {
        status = -1;
        goto handle_error;
    }
    memcpy(tmp_buffer, cjson_obj->param1->valuestring, strlen(cjson_obj->param1->valuestring) + 1);
    tmp_label->labels.param1 = tmp_buffer;

    /* Duplicate the param2 label string */
    tmp_buffer = (uint8_t*)malloc((sizeof(uint8_t) * strlen(cjson_obj->param2->valuestring)) + 1);
    
    if (tmp_buffer == NULL) {
        status = -1;
        goto handle_error;
    }
    memcpy(tmp_buffer, cjson_obj->param2->valuestring, strlen(cjson_obj->param2->valuestring) + 1);
    tmp_label->labels.param2 = tmp_buffer;


    /* Copy over the object ID */
    tmp_label->object_id = cjson_obj->value->valueint;

    status = 0;

    goto handle_return;

handle_error:
    /* Destroy the newly allocated object label and all it's pointers */
    destroy_object_label(&tmp_label);

handle_return:
    *new_label = tmp_label;
    return status;
}