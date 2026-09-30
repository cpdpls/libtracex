/**
 * @file tracex_core.c
 * @author Christos Papadopoulos (papadopoulos.chris@icloud.com)
 * @brief 
 * @version 0.1
 * @date 2026-09-30
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "tracex_core.h"
#include "tracex_errno.h"
#include "tracex_event_int.h"
#include "tracex_obj_int.h"
#include "tracex_utils.h"
#include "tracex_header.h"
#include "tracex_event.h"
#include "tracex_object.h"
#include "tracex_obj_str.h"
#include "tracex_event_str.h"

/* Flag set to detect wether tracex has been init */
static uint8_t tracex_init_done;


struct tracex_core_event_labels_json {
	cJSON *array;
	cJSON *value;
	cJSON *name;
	cJSON *param1;
	cJSON *param2;
	cJSON *param3;
	cJSON *param4;
};

struct tracex_core_event_json {
	struct tracex_core_event_labels_json event_types;
	cJSON *root;
	cJSON *tmp_item;
};



/**
 * @brief 	Internal callback that is always called, when the header has been parsed. It will call the
 * 		the user provided callback if any.
 * 
 * @param[out] cb_data The context from which the header belong to. In this case, the actual handler
 * @param[out] header  The newly parsed header, if successfull
 * @param[out] status  Status of the operation
 */
static void core_on_header_parsed_callback(void *cb_data, struct tracex_header *header, tracex_ret_t status);

/**
 * @brief 	Internal callback that is always called, when an object has been parsed. It will call the
 * 		the user provided callback if any.
 * 
 * @param[out] cb_data The context from which the object belong to. In this case, the actual handler
 * @param[out] header  The newly parsed object, if successfull
 * @param[out] status  Status of the operation
 */
static void core_on_object_parsed_callback(void *cb_data, struct tracex_object *object, tracex_ret_t status);

/**
 * @brief 	Internal callback that is always called, when an event has been parsed. It will call the
 * 		the user provided callback if any.
 * 
 * @param[out] cb_data The context from which the event belong to. In this case, the actual handler
 * @param[out] header  The newly parsed event, if successfull
 * @param[out] status  Status of the operation
 */
static void core_on_event_parsed_callback(void *cb_data, struct tracex_event *event, tracex_ret_t status);

/**
 * @brief Function that assigns the internal callback to a provided handler object
 * 
 * @param[out] handler Handler pointer that will get it's callback assign
 */
static void assign_internal_callbacks(tracex_handler_t *handler);

tracex_ret_t tracex_init(void)
{
	tracex_ret_t status;

	/*
	 * First try to detect the endianess of the host system
	 * Since a tracex dump can be in either big or little endian format
	 * We first need to detect the host endianess in order to parse correctly
	 * Do it as soon as possible
	 */
	tracex_utils_detect_indianess();

	/*
	 * Load all the default labels for the objects (object name and it's param labels)
	 * We pass NULL in order to load the default json file.
	 */
	status = tracex_object_load_labels(NULL);

	/*
	 * Load all the default labels for the events (event name and it's infos labels)
	 * We pass NULL in order to load the default json file.
	 */
	status = tracex_event_load_labels(NULL);

	tracex_init_done = 1;

early_return:
	return status;
}

void tracex_deinit(void)
{
	/* Call the destructors of the previous parsed labels from a json file */
	tracex_object_destroy_labels();
	tracex_event_destroy_labels();
}

tracex_ret_t tracex_create_new_handler(tracex_handler_t **new_handler)
{
	struct tracex_handler *handler = NULL;
	tracex_ret_t status;

	/* Don't go furthur is the user has call the init function of the library  */
	if (!tracex_init_done) {
		status = TRACEX_NOT_INIT;
		goto handle_exit;
	}

	/* Sanitize for bad input */
	if (new_handler == NULL) {
		status = TRACEX_BAD_INPUT_PTR;
		goto handle_exit;
	}

	/* Allocate a new empty handler for the user */
	handler = (struct tracex_handler *)malloc(sizeof(struct tracex_handler));

	if (handler == NULL) {
		status = TRACEX_ALLOC_FAILURE;
		goto handle_exit;
	}

	/* Faster to memset the structure to 0 */
	memset(handler, 0, sizeof(struct tracex_handler));

	/* Init the different contexts parts */
	if ((status = tracex_object_int_init(&handler->objs_ctx)) != TRACEX_SUCCESS) {
		status = TRACEX_INIT_FAILURE;
		goto handle_error;
	}

	if ((status = tracex_event_int_init(&handler->event_ctx)) != TRACEX_SUCCESS) {
		status = TRACEX_INIT_FAILURE;
		goto handle_error;
	}

	/* Last thing to do is to assign the internal callbacks */
	assign_internal_callbacks(handler);

	/* If we reached here, it means the operation was successfulll */
	status = TRACEX_SUCCESS;
	goto handle_exit;

handle_error:
	/* Destroy the previously tmp handler */
	tracex_destroy_handler(&handler);
	handler = NULL;

handle_exit:

	/* Giuve back the newly created or destroyed handler and return the status */
	*new_handler = handler;
	return status;
}

tracex_ret_t tracex_register_callbacks(tracex_handler_t *handler, struct tracex_callbacks *callbacks)
{
	/* Sanitize for the inputs */
	if (callbacks == NULL || handler == NULL)
		return TRACEX_BAD_INPUT_PTR;

	/* Assign the user provided callbacks to the handler regardless of they are valid or not */
	handler->user_callbacks.on_header_parsed = callbacks->on_header_parsed;
	handler->user_callbacks.on_object_parsed = callbacks->on_object_parsed;
	handler->user_callbacks.on_event_parsed = callbacks->on_event_parsed;

	return TRACEX_SUCCESS;
}

void tracex_destroy_handler(tracex_handler_t **handler)
{
	/* Sanitize for bad input */
	if (handler != NULL && *handler != NULL) {

		/* Destroy the object context */
		tracex_object_destroy_context(&(*handler)->objs_ctx);

		/* Destroy the event context */
		tracex_event_destroy_context(&(*handler)->event_ctx);

		/* Zero-out the whole handler, erasing subsequent contexts too */
		memset((*handler), 0, sizeof(struct tracex_handler));
		free(*handler);
		*handler = NULL;
	}
}

tracex_ret_t tracex_parse(tracex_handler_t *handler, void *buffer, size_t buffer_length, size_t *bytes_consumed)
{
	tracex_ret_t status;
	uint64_t consumed_by_phase = 0;	/* Saved number of parsed bytes per parsing phase */
	uint64_t consumed_by_call = 0;	/* Saved number of parsed bytes by adding each consumed_by_phase */
	struct tracex_header_context *hdr_ctx;

	/* Sanitize for bad user input */
	if (handler == NULL || buffer == NULL || bytes_consumed == NULL) {
		status = TRACEX_BAD_INPUT_PTR;
		goto handle_exit;
	}

	if (buffer_length == 0) {
		status = TRACEX_NULL_LENGTH;
		goto handle_exit;
	}

	hdr_ctx = &handler->hdr_ctx;

	/*
	 * In case of stream based parsing (mulitiple calls to this function),
	 * Don't process furthur if the previous call to this function declared that the header was invalid
	 * Otherwise, this checks always passes if this is the fist call to this funciton or the user it parsing
	 * the whole trace at once
	 */

	if (hdr_ctx->header_parsed && !hdr_ctx->header_valid) {
		status = TRACEX_HEADER_NOT_VALID;
		goto handle_exit;
	}

	/* Try to parse the first phase, which is the header */
	if (handler->state == E_HEADER_PHASE) {

		/* Call the actual header parsing function */
		status = tracex_header_int_parse(hdr_ctx, buffer, buffer_length, &consumed_by_phase);

		if (status == TRACEX_SUCCESS) {

			/*
			 * We now, know more about the registry size of the objects and events 
			 * So let's update those for each context and continue with the parsing
			 */
			handler->objs_ctx.name_size = handler->hdr_ctx.header.obj_registry_name_size;
			handler->objs_ctx.registry_size = handler->hdr_ctx.obj_registry_size;
			handler->event_ctx.registry_size = handler->hdr_ctx.event_registry_size;

			/* Go to the next phase, which is parsing the objects */
			handler->state = E_OBJECT_PHASE;

			/* Increment the buffer pointer and decrement the bytes left to parse */
			buffer += consumed_by_phase;
			buffer_length -= consumed_by_phase;
			
		}

		/*
		 * Even if we are going to the next phase or a failed called happened,
		 * keep the number of bytes parsed updated 
		 */
		handler->raw_bytes_count += consumed_by_phase;
		consumed_by_call += consumed_by_phase;
	}

	/* Object parsing phase  */
	if (handler->state == E_OBJECT_PHASE) {

		/*
		 * In case no more bytes are left to parse, the previous parsing phase 
		 * Would return TRACEX_SUCCESS, We need to return TRACEX_NEED_MORE if
		 * there is no more bytes So that the user can call again this function with
		 * the next part of the trace in order to continue parsing
		 */
		if (buffer_length == 0) {
			status = TRACEX_NEED_MORE;
			goto handle_exit;
		}

		/* Call the actual object parsing function */
		status = tracex_object_int_parse(&handler->objs_ctx, buffer, buffer_length, &consumed_by_phase);

		if (status == TRACEX_SUCCESS) {
			handler->state = E_EVENT_PHASE;

			/* Increment the buffer pointer and decrement the bytes left to parse */
			buffer += consumed_by_phase;
			buffer_length -= consumed_by_phase;

			/* Same as the header phase step for the remaining byte mitigation */

			if (buffer_length == 0)
				status = TRACEX_NEED_MORE;
		}

		/*
		 * Even if we are going to the next phase or a failed called happened,
		 * keep the number of bytes parsed updated 
		 */
		handler->raw_bytes_count += consumed_by_phase;
		consumed_by_call += consumed_by_phase;
	}

	/* Event parsing phase */
	if (handler->state == E_EVENT_PHASE) {

		/*
		 * In case no more bytes are left to parse, the previous parsing phase 
		 * Would return TRACEX_SUCCESS, We need to return TRACEX_NEED_MORE if
		 * there is no more bytes So that the user can call again this function with
		 * the next part of the trace in order to continue parsing
		 */
		if (buffer_length == 0) {
			status = TRACEX_NEED_MORE;
			goto handle_exit;
		}

		/* Call the actual event parsing function */
		status = tracex_event_int_parse(&handler->event_ctx, buffer, buffer_length, &consumed_by_phase);


		/*
		 * When this function retuns TRACEX_SUCCESS, it means the event buffer has
		 * been fully parsed. It also means that we are now parsing object back again
		 * in case of a streaming based parsing. The user can decide to continue parsing
		 * a buffer even if the full event buffer has been parsed.
		 * There is also no need to check if there are remaining bytes for the next
		 * phase as we are not in a loop and it's only on the next call of this function
		 * that we are parsing the objects.
		 */
		if (status == TRACEX_SUCCESS) {

			handler->state = E_OBJECT_PHASE;

			/* Increment the buffer pointer and decrement the bytes left to parse */
			buffer += consumed_by_phase;
			buffer_length -= consumed_by_phase;
		}


		/*
		 * Even if we are going to the next phase or a failed called happened,
		 * keep the number of bytes parsed updated 
		 */
		consumed_by_call += consumed_by_phase;
		handler->raw_bytes_count += consumed_by_phase;
	}

handle_exit:

	/* Assign the total consumed bytes for this call to the user provided pointer s*/
	*bytes_consumed = consumed_by_call;
	return status;
}


static void core_on_header_parsed_callback(void *cb_data, struct tracex_header *header, tracex_ret_t status)
{
	struct tracex_handler *tmp_handler;

	/* Cast the void cb_data to a handler type */
	tmp_handler = TO_HANDLER(cb_data);

	/* If the user registered it's callback when a header is parsed, call it !*/
	if (tmp_handler->user_callbacks.on_header_parsed != NULL)
		tmp_handler->user_callbacks.on_header_parsed(tmp_handler, header, status);
}
static void core_on_object_parsed_callback(void *cb_data, struct tracex_object *object, tracex_ret_t status)
{
	struct tracex_handler *tmp_handler;

	/* Cast the void cb_data to a handler type */
	tmp_handler = TO_HANDLER(cb_data);

	/* If the user registered it's callback when a header is parsed, call it !*/
	if (tmp_handler->user_callbacks.on_object_parsed != NULL)
		tmp_handler->user_callbacks.on_object_parsed(tmp_handler, object, status);
}
static void core_on_event_parsed_callback(void *cb_data, struct tracex_event *event, tracex_ret_t status)
{
	struct tracex_handler *tmp_handler;

	/* Cast the void cb_data to a handler type */
	tmp_handler = TO_HANDLER(cb_data);

	/* If the user registered it's callback when a header is parsed, call it !*/
	if (tmp_handler->user_callbacks.on_event_parsed != NULL)
		tmp_handler->user_callbacks.on_event_parsed(tmp_handler, event, status);
}

static void assign_internal_callbacks(tracex_handler_t *handler)
{
	/* Assign the callback of each handler context to the internal one */
	handler->hdr_ctx.on_header_parsed = core_on_header_parsed_callback;
	handler->objs_ctx.on_object_parsed = core_on_object_parsed_callback;
	handler->event_ctx.on_event_parsed = core_on_event_parsed_callback;

	/* Assign the callback data to be the handler pointer for later dispatch to the user */
	handler->hdr_ctx.cb_data = (void *)handler;
	handler->objs_ctx.cb_data = (void *)handler;
	handler->event_ctx.cb_data = (void *)handler;
}