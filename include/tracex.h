#ifndef __TRACEX_H__
#define __TRACEX_H__

#include <stdint.h>
#include <stddef.h>
#include "tracex_errno.h"
#include "tracex_header.h"
#include "tracex_event.h"
#include "tracex_object.h"

typedef struct TRACEX_handler_t TRACEX_handler_t;

/**
 * @brief   Function that performs internal initialization. Must always be called before any other
 *          API is called
 * 
 */
void TRACEX_INIT();

/**
 * @brief This function is used to parse data either in an incremental way. The caller can
 *        Pass an undefined number of bytes, which are going to be parsed on the go thanks
 *        to an internal State machine.
 * 
 * @param parser        The previously created TRACEX_handler_t handler.
 * @param buffer        Pointer to the buffer where the parser will read from and parse.
 * @param buffer_length The buffer length.
 * @return TRACEX_Ret_t TRACEX_SUCCESS on success, other values of TRACEX_Ret_t otherwise.
 *          Caller should call TRACEX_strerror in order to get the string value of the error.
 *
 */
TRACEX_Ret_t TRACEX_parse(TRACEX_handler_t *handler, void *buffer, size_t buffer_length);

/**
 * @brief Creates a TRACEX_handler_t, which is the core context for the other API's.
 * 
 * @return TRACEX_handler_t on success, NULL otherwise.
 */
TRACEX_Ret_t TRACEX_createHandler(struct TRACEX_handler_t **handler_ptr);

/**
 * @brief Destroys a handler allocated previously.
 * 
 * @param handler the previoulsy allocated handler.
 */
void TRACEX_destroyHandler(TRACEX_handler_t **handler);


#endif