#ifndef __TRACEX_HEADER_H__
#define __TRACEX_HEADER_H__

#include <stdint.h>
#include "tracex_errno.h"

typedef struct TRACEX_handler_t TRACEX_handler_t;       /* Forward declaration */

enum TRACEX_dump_endianess_t
{
    E_TRACEX_LITTLE_ENDIAN,
    E_TRACEX_BIG_ENDIAN,
};

struct TRACEX_header_t
{
    const uint32_t                        timer_mask;
    const enum TRACEX_dump_endianess_t    endianess;
    const uint8_t                         object_name_size;
};

/**
 * @brief Returns wether the header has been parsed yet at any given time.
 * 
 * @param TRACEX_handler_t Pointer to the previously allocated handler. 
 * @return  TRACEX_Ret_t TRACEX_SUCCESS on success, TRACEX_NEED_MORE if more bytes are required,
 *          Other value from TRACEX_Ret_t otherwise.
 */
TRACEX_Ret_t TRACEX_isHeaderParsed(struct TRACEX_handler_t *handler);

/**
 * @brief Returns wether the header is a valid TraceX header.
 * 
 * @param TRACEX_handler_t Pointer to the previously allocated handler. 
 * @return TRACEX_Ret_t TRACEX_Ret_t TRACEX_SUCCESS on success, TRACEX_HEADER_NOT_VALID if the header is invalid,
 *          Other value from TRACEX_Ret_t otherwise.
 */
TRACEX_Ret_t TRACEX_isHeaderValid(struct TRACEX_handler_t *handler);

/**
 * @brief returns the parsed header if already parsed
 * 
 * @param handler Pointer to the previously allocated handler.
 * @param header  pointer to a memory location where the parsed header will be returned
 *                to the user.
 * @return TRACEX_Ret_t TRACEX_SUCCESS on success, TRACEX_NEED_MORE if more bytes are required,
 *          Other value from TRACEX_Ret_t otherwise.
 */
TRACEX_Ret_t TRACEX_getHeader(struct TRACEX_handler_t *handler, struct TRACEX_header_t **header);
#endif