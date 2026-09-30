#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "uthash.h"
#include "tracex_event_str.h"
#include "cJSON.h"
#include "tracex_list.h"
#include "tracex_utils.h"

/* Default path of the json file shiped with this library */
#define TRACEX_EVENT_DEFAULT_JSON_PATH "data/events.json"

/* In case an info label for a given event ID does not exist,
 * This string will be assigned to the event
 */

static uint8_t *default_invalid_info = "Not valid";
/* In case a label for a given event ID does not exist,
 * This string will be assigned to the event
 */
static uint8_t *default_invalid_event = "Invalid";

enum label_add_override_setting {
	E_LABEL_ADD_NO_OVERRIDE,
	E_LABEL_ADD_OVERRIDE,
};

struct tracex_event_json_element {
	cJSON *value;
	cJSON *name;
	cJSON *info1;
	cJSON *info2;
	cJSON *info3;
	cJSON *info4;
};

struct tracex_event_json {
	struct tracex_event_json_element event_element;
	cJSON *root;
	cJSON *array;
	cJSON *tmp_cjson_elem_ptr;
};

struct tracex_event_labels {
	uint8_t *eventLabel;
	uint32_t eventId;
	struct tracex_event_infos_labels infoLabels;

	UT_hash_handle hh;
};

/* Global variable that holds all the parsed labels from a json file using a hash table */
static struct tracex_event_labels *event_labels_hash = NULL;

/**
 * @brief Allocated a number of struct tracex_event_labels in a contiguous memory address space
 * 
 * @param count how many struct tracex_event_labels to allocated
 * @return struct tracex_event_labels* Address to the allocated memory
 */
static struct tracex_event_labels *alloc_event_labels(size_t count);

/**
 * @brief Destroys a previously allocated struct tracex_event_labels
 * 
 * @param event_label the address that points to an event label
 */
static void destroy_event_label(struct tracex_event_labels **event_label);

/**
 * @brief Adds a previously created event label structure to the global list of event labels. If
 *          The label exists and override is set, the newly to be added label will replace the existing one.
 * 
 * @param new_labels pointer to the label that should be added to the list 
 */
static void tracex_event_labels_add(struct tracex_event_labels *new_labels, enum label_add_override_setting override);

/**
 * @brief Converts a json label entry to compatible struct tracex_event_labels. This function dynamically allocated a new
 * 	  struct tracex_event_labels.
 * 	 
 * 
 * @param cjson_event the json label entry
 * @param new_label Pointer where the newly created event label is allocated.
 * @return tracex_ret_t 
 */
static tracex_ret_t convert_cjson_to_event_label(struct tracex_event_json_element *cjson_event,
						 struct tracex_event_labels **new_label);

const uint8_t *tracex_event_id_to_str(uint32_t id)
{
	struct tracex_event_labels *entry = NULL;
	uint8_t *string = NULL;


	/* First check if the provided ID corresponds to a set of previously parsed string labels from the json file */
	HASH_FIND_INT(event_labels_hash, &id, entry);

	/* If the ID is part of the hashed list, we can return it's event label string */
	if (entry)
		string = entry->eventLabel;
	
	/* Othwersise, the ID is not linked to any existing label parsed from the json. Assign it the default string */
	else
		string = default_invalid_event;

	return string;
}

struct tracex_event_infos_labels tracex_event_infos_to_str(uint32_t id)
{
	struct tracex_event_infos_labels infos;
	struct tracex_event_labels *entry = NULL;


	/* First check if the provided ID corresponds to a set of previously parsed string labels from the json file */

	HASH_FIND_INT(event_labels_hash, &id, entry);

	if (entry) {
		infos.info1_label = entry->infoLabels.info1_label;
		infos.info2_label = entry->infoLabels.info2_label;
		infos.info3_label = entry->infoLabels.info3_label;
		infos.info4_label = entry->infoLabels.info4_label;

	} else {
		/* Set the params to invalid in case of an error */
		infos.info1_label = default_invalid_info;
		infos.info2_label = default_invalid_info;
		infos.info3_label = default_invalid_info;
		infos.info4_label = default_invalid_info;
	}
	return infos;
}

tracex_ret_t tracex_event_load_labels(uint8_t *json_path)
{
	tracex_ret_t status;
	struct tracex_event_json json_struct;
	struct tracex_event_labels *labels_ptr = NULL;
	uint8_t *actual_path = NULL;

	memset(&json_struct, 0, sizeof(struct tracex_event_json));

	if (json_path == NULL)
		actual_path = TRACEX_EVENT_DEFAULT_JSON_PATH;
	else
		actual_path = json_path;

	status = tracex_utils_load_json_file(actual_path, &json_struct.root);

	if (status != TRACEX_SUCCESS) {
		goto handle_return;
	}

	/* The goal here is to get get each element that is in the array of eventTypesNames
     * Inside the JSON file and 1 struct tracex_event_labels in oder to add that element
     * In the list.
     */

	json_struct.array = cJSON_GetObjectItemCaseSensitive(json_struct.root, "eventTypesNames");

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

	/* Iterate on all the element of the array and convert it to a struct tracex_event_labels */
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

		/* Convert the cJSON event to a struct tracex_event_labels and add it */
		if (convert_cjson_to_event_label(&json_struct.event_element, &labels_ptr) == TRACEX_SUCCESS) {
			/* We finally add the converted tracex_event_labels to the global list */
			tracex_event_labels_add(labels_ptr, E_LABEL_ADD_OVERRIDE);
		}
	}

handle_return:

	/*  Free the parsed json event as we don't need them anymore
    *   They have been converted to struct tracex_event_labels and are part of a list
    */

	if (json_struct.root != NULL) {
		cJSON_Delete(json_struct.root);
	}
	return status;
}

void tracex_event_destroy_labels(void)
{
	struct tracex_event_labels *entry, *tmp;

	/* Walk the hash table and free everything */
	HASH_ITER(hh, event_labels_hash, entry, tmp)
	{
		destroy_event_label(&entry);
	}
}

static void tracex_event_labels_add(struct tracex_event_labels *new_labels, enum label_add_override_setting override)
{
	struct tracex_event_labels *existing = NULL;
	uint8_t found = 0;

	HASH_FIND_INT(event_labels_hash, &new_labels->eventId, existing);

	/* If the event is found and override is allowed, delete the existing*/
	if (existing) {
		if (override == E_LABEL_ADD_OVERRIDE) {
			destroy_event_label(&new_labels);
		} else
			goto early_return;
	}

early_add_list:

	/* Add the actual element in the list */
	HASH_ADD_INT(event_labels_hash, eventId, new_labels);

early_return:
	return;
}

static void destroy_event_label(struct tracex_event_labels **event_label)
{
	if (event_label != NULL) {
		if (*event_label != NULL) {
			/* Destroy the event type string */
			if ((*event_label)->eventLabel != NULL) {
				free((*event_label)->eventLabel);
				(*event_label)->eventLabel = NULL;
			}

			/* Destroy the event info 1 string */
			if ((*event_label)->infoLabels.info1_label != NULL) {
				free((void *)(*event_label)->infoLabels.info1_label);
				(*event_label)->infoLabels.info1_label = NULL;
			}

			/* Destroy the event info 2 string */
			if ((*event_label)->infoLabels.info2_label != NULL) {
				free((void *)(*event_label)->infoLabels.info2_label);
				(*event_label)->infoLabels.info2_label = NULL;
			}

			/* Destroy the event info 3 string */
			if ((*event_label)->infoLabels.info3_label != NULL) {
				free((void *)(*event_label)->infoLabels.info3_label);
				(*event_label)->infoLabels.info3_label = NULL;
			}

			/* Destroy the event info 4 string */
			if ((*event_label)->infoLabels.info4_label != NULL) {
				free((void *)(*event_label)->infoLabels.info4_label);
				(*event_label)->infoLabels.info4_label = NULL;
			}

			HASH_DEL(event_labels_hash, *event_label);

			free(*event_label);
			*event_label = NULL;
		}
	}
}

static struct tracex_event_labels *alloc_event_labels(size_t count)
{
	struct tracex_event_labels *tmp = NULL;

	if (count == 0)
		goto early_return;

	tmp = (struct tracex_event_labels *)malloc(sizeof(struct tracex_event_labels) * count);

	if (tmp == NULL)
		goto early_return;

early_return:
	return tmp;
}

static tracex_ret_t convert_cjson_to_event_label(struct tracex_event_json_element *cjson_event,
						 struct tracex_event_labels **new_label)
{
	struct tracex_event_labels *tmp_label = NULL;
	uint8_t *tmp_buffer = NULL;
	tracex_ret_t status;

	/* Check for any missing field */
	if (cjson_event->name == NULL || cjson_event->info1 == NULL || cjson_event->info2 == NULL ||
	    cjson_event->info3 == NULL || cjson_event->info4 == NULL) {
		status = TRACEX_BAD_INPUT_PTR;
		goto handle_return;
	}

	/* Check for a NULL string value */

	if (cjson_event->name->valuestring == NULL || cjson_event->info1->valuestring == NULL ||
	    cjson_event->info2->valuestring == NULL || cjson_event->info3->valuestring == NULL ||
	    cjson_event->info4->valuestring == NULL) {
		status = TRACEX_JSON_PARSING_FAILURE;
		goto handle_return;
	}

	/* Alloc a new event label */
	tmp_label = alloc_event_labels(1);

	/* Check if successfull */
	if (tmp_label == NULL) {
		status = TRACEX_ALLOC_FAILURE;
		goto handle_return;
	}

	/* Erase the newly allocated event labels */
	memset(tmp_label, 0, sizeof(struct tracex_event_labels));

	/* Duplicate the Event ID label string  */
	tmp_buffer = (uint8_t *)malloc((sizeof(uint8_t) * strlen(cjson_event->name->valuestring)) + 1);

	if (tmp_buffer == NULL) {
		status = TRACEX_ALLOC_FAILURE;
		goto handle_error;
	}

	memcpy(tmp_buffer, cjson_event->name->valuestring, strlen(cjson_event->name->valuestring) + 1);
	tmp_label->eventLabel = tmp_buffer;

	/* Duplicate the info1 label string */
	tmp_buffer = (uint8_t *)malloc((sizeof(uint8_t) * strlen(cjson_event->info1->valuestring)) + 1);

	if (tmp_buffer == NULL) {
		status = TRACEX_ALLOC_FAILURE;
		goto handle_error;
	}
	memcpy(tmp_buffer, cjson_event->info1->valuestring, strlen(cjson_event->info1->valuestring) + 1);
	tmp_label->infoLabels.info1_label = tmp_buffer;

	/* Duplicate the info2 label string */
	tmp_buffer = (uint8_t *)malloc((sizeof(uint8_t) * strlen(cjson_event->info2->valuestring)) + 1);

	if (tmp_buffer == NULL) {
		status = TRACEX_ALLOC_FAILURE;
		goto handle_error;
	}
	memcpy(tmp_buffer, cjson_event->info2->valuestring, strlen(cjson_event->info2->valuestring) + 1);
	tmp_label->infoLabels.info2_label = tmp_buffer;

	/* Duplicate the info3 label string */
	tmp_buffer = (uint8_t *)malloc((sizeof(uint8_t) * strlen(cjson_event->info3->valuestring)) + 1);

	if (tmp_buffer == NULL) {
		status = TRACEX_ALLOC_FAILURE;
		goto handle_error;
	}
	memcpy(tmp_buffer, cjson_event->info3->valuestring, strlen(cjson_event->info3->valuestring) + 1);
	tmp_label->infoLabels.info3_label = tmp_buffer;

	/* Duplicate the info4 label string */
	tmp_buffer = (uint8_t *)malloc((sizeof(uint8_t) * strlen(cjson_event->info4->valuestring)) + 1);

	if (tmp_buffer == NULL) {
		status = TRACEX_ALLOC_FAILURE;
		goto handle_error;
	}
	memcpy(tmp_buffer, cjson_event->info4->valuestring, strlen(cjson_event->info4->valuestring) + 1);
	tmp_label->infoLabels.info4_label = tmp_buffer;

	/* Copy over the event ID */
	tmp_label->eventId = cjson_event->value->valueint;

	status = TRACEX_SUCCESS;

	goto handle_return;

handle_error:
	/* Destroy the newly allocated event label and all it's pointers */
	destroy_event_label(&tmp_label);

handle_return:
	*new_label = tmp_label;
	return status;
}