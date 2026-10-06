#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "uthash.h"
#include "cJSON.h"
#include "tracex/tracex_resolver.h"
#include "labels_engine/labels_engine.h"
#include "labels_engine_utils.h"

/* Default path of the json file shiped with this library for the objects labels */
#define LABELS_ENGINE_DEFAULT_OBJECT_JSON_PATH "data/objects.json"

/* Default path of the json file shiped with this library for the events labels */
#define LABELS_ENGINE_DEFAULT_EVENT_JSON_PATH "data/events.json"


struct labels_engine_object_json_element {
	cJSON   *value;                
    cJSON   *name;
    cJSON   *param1;
    cJSON   *param2;
};

struct labels_engine_object_json {

	struct labels_engine_object_json_element 	obj_element;
    cJSON                                   	*root;
    cJSON                                   	*array;
    cJSON                                   	*tmp_cjson_elem_ptr;
};

struct labels_engine_object_labels {
    uint32_t object_id;                         /* The object ID as found in the enum tracex_object_type */
    struct tracex_object_labels labels; 		/* The object strings for the param 1 and param 2 */

    UT_hash_handle hh;                           /* The next node */
};

struct labels_engine_event_json_element {
	cJSON *value;
	cJSON *name;
	cJSON *info1;
	cJSON *info2;
	cJSON *info3;
	cJSON *info4;
};

struct labels_engine_event_json {

	struct labels_engine_event_json_element event_element;
	cJSON 									*root;
	cJSON 									*array;
	cJSON 									*tmp_cjson_elem_ptr;
};

struct labels_engine_event_labels {
	uint32_t eventId;
	struct tracex_event_labels labels;

	UT_hash_handle hh;
};

/* Global variable that holds all the parsed object labels from a json file using a hash table */
static struct labels_engine_object_labels *object_labels_hash = NULL;


/* Global variable that holds all the parsed event labels from a json file using a hash table */
static struct labels_engine_event_labels *event_labels_hash = NULL;

/**
 * @brief Allocated a number of struct labels_engine_event_labels in a contiguous memory address space
 * 
 * @param count how many struct labels_engine_event_labels to allocated
 * @return struct labels_engine_event_labels* Address to the allocated memory
 */
static struct labels_engine_event_labels *alloc_event_labels(size_t count);

/**
 * @brief Destroys a previously allocated struct labels_engine_event_labels
 * 
 * @param event_label the address that points to an event label
 */
static void destroy_event_label(struct labels_engine_event_labels **event_label);

/**
 * @brief Adds a previously created event label structure to the global list of event labels. If
 *          The label exists and override is set, the newly to be added label will replace the existing one.
 * 
 * @param new_labels pointer to the label that should be added to the list 
 */
static void labels_engine_event_labels_add(struct labels_engine_event_labels *new_labels);

/**
 * @brief Converts a json label entry to compatible struct labels_engine_event_labels. This function dynamically allocated a new
 * 	  struct labels_engine_event_labels.
 * 	 
 * 
 * @param cjson_event the json label entry
 * @param new_label Pointer where the newly created event label is allocated.
 * @return tracex_labels_ret_t 
 */
static int convert_cjson_to_event_label(struct labels_engine_event_json_element *cjson_event,
						 struct labels_engine_event_labels **new_label);


static struct labels_engine_object_labels *alloc_object_labels(size_t count);
static void destroy_object_label(struct labels_engine_object_labels **obj_label);
static void labels_engine_object_labels_add(struct labels_engine_object_labels *new_labels);
static int convert_cjson_to_obj_label(struct labels_engine_object_json_element *cjson_obj, struct labels_engine_object_labels **new_label);


tracex_resolver_labels labels_engine_resolve_labels(enum tracex_resolver_request_type request, uint32_t label_id)
{
	tracex_resolver_labels labels;
	struct labels_engine_object_labels *entry_object = NULL;
	struct labels_engine_event_labels *entry_event = NULL;

	if (request == E_TRACEX_RESOLVER_OBJECT) {
		/* First check if the provided type corresponds to a set of previously parsed object labels from the json file */
    	HASH_FIND_INT(object_labels_hash, &label_id, entry_object);

		/* If the type is part of the hashed list, we can return the object labels  */
		if (entry_object) {
			labels.objLabels = entry_object->labels;
		} else {
			memset(&labels, 0, sizeof(tracex_resolver_labels));
		}
		/* Othwersise, the type is not linked to any existing label parsed from the json. Assign it the default string */
	}

	if (request == E_TRACEX_RESOLVER_EVENT) {
		/* First check if the provided type corresponds to a set of previously parsed object labels from the json file */
    	HASH_FIND_INT(event_labels_hash, &label_id, entry_event);

		/* If the type is part of the hashed list, we can return the object labels  */
		if (entry_event) {
			labels.eventLabels = entry_event->labels;
		} else {
			memset(&labels, 0, sizeof(tracex_resolver_labels));
		}
		/* Othwersise, the type is not linked to any existing label parsed from the json. Assign it the default string */
	}

	return labels;
}

// struct tracex_object_labels tracex_resolver_get_object_labels(uint32_t type)
// {
//     struct tracex_labels_object_labels *entry = NULL;
//     struct tracex_object_labels labels;

//     /* First check if the provided type corresponds to a set of previously parsed object labels from the json file */
//     HASH_FIND_INT(object_labels_hash, &type, entry);

//     /* If the type is part of the hashed list, we can return the object labels  */
// 	if (entry)
// 		labels = entry->labels;

//     /* Othwersise, the type is not linked to any existing label parsed from the json. Assign it the default string */
//     else
//         memset(&labels, 0, sizeof(struct tracex_object_labels));

//     return labels;
// }


// tracex_resolver_labels labels_engine_resolve_labels(enum tracex_resolver_request_type request, uint32_t label_id)
// {
// 	struct labels_engine_event_labels *entry = NULL;
// 	struct tracex_event_labels labels;

// 	/* First check if the provided ID corresponds to a set of previously parsed event labels from the json file */
// 	HASH_FIND_INT(event_labels_hash, &event_id, entry);

// 	/* If the ID is part of the hashed list, we can return it's event label strings */
// 	if (entry)
// 		labels = entry->labels;

// 	/* Othwersise, the ID is not linked to any existing label parsed from the json. Assign it the default string */
// 	else
// 		memset(&labels, 0, sizeof(struct tracex_event_labels));

// 	return labels;
// }

int labels_engine_object_load_labels(const char *json_path)
{
    int status;
    struct labels_engine_object_json json_struct;
    struct labels_engine_object_labels *labels_ptr = NULL;
    const char *actual_path = NULL;

    memset(&json_struct, 0, sizeof(struct labels_engine_object_json));

    if (json_path == NULL)
        actual_path = LABELS_ENGINE_DEFAULT_OBJECT_JSON_PATH;  
    else
        actual_path = json_path;

    status = labels_engine_utils_load_json_file(actual_path, &json_struct.root);

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
            labels_engine_object_labels_add(labels_ptr);
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

void labels_engine_object_destroy_labels(void)
{
    struct labels_engine_object_labels *entry = NULL;
    struct labels_engine_object_labels *tmp = NULL;

    /* Walk the hash table and free everything */
	HASH_ITER(hh, object_labels_hash, entry, tmp)
	{
		destroy_object_label(&entry);
	}
}

static void labels_engine_object_labels_add(struct labels_engine_object_labels *new_labels)
{
    struct labels_engine_object_labels *existing = NULL;
    uint8_t found = 0;

    HASH_FIND_INT(object_labels_hash, &new_labels->object_id, existing);

    /* If the event is found and override is allowed, delete the existing*/
	if (existing) {
			destroy_object_label(&new_labels);
	}

	/* Add the actual element in the list */
	HASH_ADD_INT(object_labels_hash, object_id, new_labels);

early_return:
	return;
}

static void destroy_object_label(struct labels_engine_object_labels **obj_label)
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

static struct labels_engine_object_labels *alloc_object_labels(size_t count)
{
    struct labels_engine_object_labels *tmp = NULL;

    if (count == 0)
        goto early_return;
    
    tmp = (struct labels_engine_object_labels*)malloc(sizeof(struct labels_engine_object_labels) * count);

    if (tmp == NULL)
        goto early_return;
    

early_return:
    return tmp;

}

static int convert_cjson_to_obj_label(struct labels_engine_object_json_element *cjson_obj, struct labels_engine_object_labels **new_label)
{
    int status;
    struct labels_engine_object_labels *tmp_label = NULL;
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
    memset(tmp_label, 0, sizeof(struct labels_engine_object_labels));

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


int labels_engine_event_load_labels(const char *json_path)
{
	int status;
	struct labels_engine_event_json json_struct;
	struct labels_engine_event_labels *labels_ptr = NULL;
	const char *actual_path = NULL;

	memset(&json_struct, 0, sizeof(struct labels_engine_event_json));

	if (json_path == NULL)
		actual_path = LABELS_ENGINE_DEFAULT_EVENT_JSON_PATH;
	else
		actual_path = json_path;

	status = labels_engine_utils_load_json_file(actual_path, &json_struct.root);

	if (status != 0) {
		goto handle_return;
	}

	/* The goal here is to get get each element that is in the array of eventTypesNames
     * Inside the JSON file and 1 struct labels_engine_event_labels in oder to add that element
     * In the list.
     */

	json_struct.array = cJSON_GetObjectItemCaseSensitive(json_struct.root, "eventTypesNames");

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

	/* Iterate on all the element of the array and convert it to a struct labels_engine_event_labels */
	cJSON_ArrayForEach(json_struct.tmp_cjson_elem_ptr, json_struct.array)
	{
		/* Get the value field */
		json_struct.event_element.value =
			cJSON_GetObjectItemCaseSensitive(json_struct.tmp_cjson_elem_ptr, "value");
		/* Get the name string value */
		json_struct.event_element.name =
			cJSON_GetObjectItemCaseSensitive(json_struct.tmp_cjson_elem_ptr, "name");
		/* Get the different params */
		json_struct.event_element.info1 =
			cJSON_GetObjectItemCaseSensitive(json_struct.tmp_cjson_elem_ptr, "info1");
		json_struct.event_element.info2 =
			cJSON_GetObjectItemCaseSensitive(json_struct.tmp_cjson_elem_ptr, "info2");
		json_struct.event_element.info3 =
			cJSON_GetObjectItemCaseSensitive(json_struct.tmp_cjson_elem_ptr, "info3");
		json_struct.event_element.info4 =
			cJSON_GetObjectItemCaseSensitive(json_struct.tmp_cjson_elem_ptr, "info4");

		/* Convert the cJSON event to a struct labels_engine_event_labels and add it */
		if (convert_cjson_to_event_label(&json_struct.event_element, &labels_ptr) == 0) {
			/* We finally add the converted labels_engine_event_labels to the global list */
			labels_engine_event_labels_add(labels_ptr);
		}
	}

	status = 0;

handle_return:

	/*  Free the parsed json event as we don't need them anymore
    *   They have been converted to struct labels_engine_event_labels and are part of a list
    */

	if (json_struct.root != NULL) {
		cJSON_Delete(json_struct.root);
	}
	return status;
}

void labels_engine_event_destroy_labels(void)
{
	struct labels_engine_event_labels *entry = NULL;
	
	struct labels_engine_event_labels *tmp = NULL;

	/* Walk the hash table and free everything */
	HASH_ITER(hh, event_labels_hash, entry, tmp)
	{
		destroy_event_label(&entry);
	}
}

static void labels_engine_event_labels_add(struct labels_engine_event_labels *new_labels)
{
	struct labels_engine_event_labels *existing = NULL;
	uint8_t found = 0;

	HASH_FIND_INT(event_labels_hash, &new_labels->eventId, existing);

	/* If the event is found and override is allowed, delete the existing*/
	if (existing) {
			destroy_event_label(&new_labels);
	}

	/* Add the actual element in the list */
	HASH_ADD_INT(event_labels_hash, eventId, new_labels);

early_return:
	return;
}

static void destroy_event_label(struct labels_engine_event_labels **event_label)
{
	if (event_label != NULL) {
		if (*event_label != NULL) {
			/* Destroy the event name string */
			if ((*event_label)->labels.event_name != NULL) {
				free((void *)(*event_label)->labels.event_name);
				(*event_label)->labels.event_name = NULL;
			}

			/* Destroy the event info 1 string */
			if ((*event_label)->labels.info1 != NULL) {
				free((void *)(*event_label)->labels.info1);
				(*event_label)->labels.info1 = NULL;
			}

			/* Destroy the event info 2 string */
			if ((*event_label)->labels.info2 != NULL) {
				free((void *)(*event_label)->labels.info2);
				(*event_label)->labels.info2 = NULL;
			}

			/* Destroy the event info 3 string */
			if ((*event_label)->labels.info3 != NULL) {
				free((void *)(*event_label)->labels.info3);
				(*event_label)->labels.info3 = NULL;
			}

			/* Destroy the event info 4 string */
			if ((*event_label)->labels.info4 != NULL) {
				free((void *)(*event_label)->labels.info4);
				(*event_label)->labels.info4 = NULL;
			}

			HASH_DEL(event_labels_hash, *event_label);

			free(*event_label);
			*event_label = NULL;
		}
	}
}

static struct labels_engine_event_labels *alloc_event_labels(size_t count)
{
	struct labels_engine_event_labels *tmp = NULL;

	if (count == 0)
		goto early_return;

	tmp = (struct labels_engine_event_labels *)malloc(sizeof(struct labels_engine_event_labels) * count);

	if (tmp == NULL)
		goto early_return;

early_return:
	return tmp;
}

static int convert_cjson_to_event_label(struct labels_engine_event_json_element *cjson_event, struct labels_engine_event_labels **new_label)
{
	struct labels_engine_event_labels *tmp_label = NULL;
	uint8_t *tmp_buffer = NULL;
	int status;

	/* Check for any missing field */
	if (cjson_event->name == NULL || cjson_event->info1 == NULL || cjson_event->info2 == NULL ||
	    cjson_event->info3 == NULL || cjson_event->info4 == NULL) {
		status = -1;
		goto handle_return;
	}

	/* Check for a NULL string value */

	if (cjson_event->name->valuestring == NULL || cjson_event->info1->valuestring == NULL ||
	    cjson_event->info2->valuestring == NULL || cjson_event->info3->valuestring == NULL ||
	    cjson_event->info4->valuestring == NULL) {
		status = -1;
		goto handle_return;
	}

	/* Alloc a new event label */
	tmp_label = alloc_event_labels(1);

	/* Check if successfull */
	if (tmp_label == NULL) {
		status = -1;
		goto handle_return;
	}

	/* Erase the newly allocated event labels */
	memset(tmp_label, 0, sizeof(struct labels_engine_event_labels));

	/* Duplicate the Event ID label string  */
	tmp_buffer = (uint8_t *)malloc((sizeof(uint8_t) * strlen(cjson_event->name->valuestring)) + 1);

	if (tmp_buffer == NULL) {
		status = -1;
		goto handle_error;
	}

	memcpy(tmp_buffer, cjson_event->name->valuestring, strlen(cjson_event->name->valuestring) + 1);
	tmp_label->labels.event_name = tmp_buffer;

	/* Duplicate the info1 label string */
	tmp_buffer = (uint8_t *)malloc((sizeof(uint8_t) * strlen(cjson_event->info1->valuestring)) + 1);

	if (tmp_buffer == NULL) {
		status = -1;
		goto handle_error;
	}
	memcpy(tmp_buffer, cjson_event->info1->valuestring, strlen(cjson_event->info1->valuestring) + 1);
	tmp_label->labels.info1 = tmp_buffer;

	/* Duplicate the info2 label string */
	tmp_buffer = (uint8_t *)malloc((sizeof(uint8_t) * strlen(cjson_event->info2->valuestring)) + 1);

	if (tmp_buffer == NULL) {
		status = -1;
		goto handle_error;
	}
	memcpy(tmp_buffer, cjson_event->info2->valuestring, strlen(cjson_event->info2->valuestring) + 1);
	tmp_label->labels.info2 = tmp_buffer;

	/* Duplicate the info3 label string */
	tmp_buffer = (uint8_t *)malloc((sizeof(uint8_t) * strlen(cjson_event->info3->valuestring)) + 1);

	if (tmp_buffer == NULL) {
		status = -1;
		goto handle_error;
	}
	memcpy(tmp_buffer, cjson_event->info3->valuestring, strlen(cjson_event->info3->valuestring) + 1);
	tmp_label->labels.info3 = tmp_buffer;

	/* Duplicate the info4 label string */
	tmp_buffer = (uint8_t *)malloc((sizeof(uint8_t) * strlen(cjson_event->info4->valuestring)) + 1);

	if (tmp_buffer == NULL) {
		status = -1;
		goto handle_error;
	}
	memcpy(tmp_buffer, cjson_event->info4->valuestring, strlen(cjson_event->info4->valuestring) + 1);
	tmp_label->labels.info4 = tmp_buffer;

	/* Copy over the event ID */
	tmp_label->eventId = cjson_event->value->valueint;

	status = 0;

	goto handle_return;

handle_error:
	/* Destroy the newly allocated event label and all it's pointers */
	destroy_event_label(&tmp_label);

handle_return:
	*new_label = tmp_label;
	return status;
}