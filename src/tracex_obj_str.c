#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "tracex_object.h"
#include "tracex_obj_str.h"


struct tracex_list object_labels_list;

static struct tracex_object_labels *alloc_object_labels(size_t count);
static void destroy_object_label(struct tracex_object_labels **obj_label);
static struct tracex_object_labels *duplicate_object_labels(struct tracex_object_labels *orig);

const uint8_t *tracex_object_type_to_str(enum tracex_object_type type)
{

    struct tracex_object_labels *entry;
    uint8_t *string;

    string = NULL;

    tracex_list_for_each_entry(entry, &object_labels_list, node)
    {
        if (entry->object_id == type) {
            string = entry->object_type_str;
        }
    }

    return string;
}

struct tracex_object_params_str tracex_object_param_to_str(enum tracex_object_type type)
{
    struct tracex_object_params_str params;
    struct tracex_object_labels *entry;

    tracex_list_for_each_entry(entry, &object_labels_list, node)
    {
        if (entry->object_id == type) {
            params.param1_label = entry->params_str.param1_label;
            params.param2_label = entry->params_str.param2_label;
        }
    }
    return params;
}


void tracex_object_labels_add(struct tracex_object_labels *new_labels)
{

    struct tracex_object_labels *tmp_label;

    /* Check for an unknown object ID */
    if (new_labels->object_id > TRACEX_OBJECT_TYPE_MAX) {
        goto early_return;
    }

    /* In case we are adding the very first element of the list, we need to initialize the list */
    if (object_labels_list.prev == NULL || object_labels_list.next == NULL) {
        tracex_list_init(&object_labels_list);
    }

    tmp_label = duplicate_object_labels(new_labels);

    if (tmp_label == NULL)
        goto early_return;
    
    
    /* Add the actual element in the list */
    tracex_list_insert(&tmp_label->node, &object_labels_list);

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

static struct tracex_object_labels *duplicate_object_labels(struct tracex_object_labels *orig)
{
    struct tracex_object_labels *new_obj;

    if (orig->object_type_str == NULL ||
        orig->params_str.param1_label == NULL ||
        orig->params_str.param2_label == NULL)
        goto early_return;

    new_obj = alloc_object_labels(1);

    if (new_obj == NULL)
        goto early_return;
        
    /* Copy all fields even the pointers values. Theey will be overriden right after */
    memcpy(new_obj, orig, sizeof(struct tracex_object_labels));

    /* Duplicate the object type string */
    new_obj->object_type_str = (uint8_t*)malloc((sizeof(uint8_t) * strlen(orig->object_type_str)) + 1);

    if (new_obj->object_type_str == NULL)
        goto handle_error;
    
    memcpy(new_obj->object_type_str, orig->object_type_str, strlen(orig->object_type_str) + 1);

    /* Duplicate the param 1 */
    new_obj->params_str.param1_label = (uint8_t*)malloc((sizeof(uint8_t) * strlen(orig->params_str.param1_label)) + 1);

    if (new_obj->params_str.param1_label == NULL)
        goto handle_error;
    
    memcpy((void*)new_obj->params_str.param1_label, orig->params_str.param1_label, strlen(orig->params_str.param1_label) + 1);

    /* Duplicate the param 2 */
    new_obj->params_str.param2_label = (uint8_t*)malloc((sizeof(uint8_t) * strlen(orig->params_str.param2_label)) + 1);

    if (new_obj->params_str.param2_label == NULL)
        goto handle_error;
    
    memcpy((void*)new_obj->params_str.param2_label, orig->params_str.param2_label, strlen(orig->params_str.param2_label) + 1);
        
    goto early_return;

handle_error:
        destroy_object_label(&new_obj);
early_return:
    return new_obj;
}

void tracex_object_destroy_labels(void)
{
    struct tracex_object_labels *entry;
    struct tracex_object_labels *next;

    tracex_list_for_each_entry_safe(entry, next, &object_labels_list,node)
    {
        destroy_object_label(&entry);
    }
}