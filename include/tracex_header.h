#ifndef __TRACEX_HEADER_H__
#define __TRACEX_HEADER_H__

#include <stdint.h>
#include "tracex_errno.h"

typedef struct tracex_handler tracex_handler;       /* Forward declaration */

enum tracex_dump_endianess
{
    E_TRACEX_LITTLE_ENDIAN,
    E_TRACEX_BIG_ENDIAN,
};


struct tracex_header
{
    uint32_t id;
    uint32_t timestamp_mask;
    uint32_t trace_base_addr;
    uint32_t obj_registry_start_ptr;
    uint16_t res1;
    uint16_t obj_registry_name_size;
    uint32_t obj_registry_end_ptr;
    uint32_t event_buff_start_ptr;
    uint32_t event_buff_end_ptr;
    uint32_t event_buff_curr_ptr;
    uint32_t res2;
    uint32_t res3;
    uint32_t res4;
} __attribute__((__packed__));

/**
 * @brief Returns wether the header has been parsed yet at any given time.
 * 
 * @param tracex_handler Pointer to the previously allocated handler. 
 * @return  tracex_ret_t TRACEX_SUCCESS on success, TRACEX_NEED_MORE if more bytes are required,
 *          Other value from tracex_ret_t otherwise.
 */
tracex_ret_t TRACEX_isHeaderParsed(struct tracex_handler *handler);

/**
 * @brief Returns wether the header is a valid TraceX header.
 * 
 * @param tracex_handler Pointer to the previously allocated handler. 
 * @return tracex_ret_t tracex_ret_t TRACEX_SUCCESS on success, TRACEX_HEADER_NOT_VALID if the header is invalid,
 *          Other value from tracex_ret_t otherwise.
 */
tracex_ret_t TRACEX_isHeaderValid(struct tracex_handler *handler);

/**
 * @brief returns the parsed header if already parsed
 * 
 * @param handler Pointer to the previously allocated handler.
 * @param header  pointer to a memory location where the parsed header will be returned
 *                to the user.
 * @return tracex_ret_t TRACEX_SUCCESS on success, TRACEX_NEED_MORE if more bytes are required,
 *          Other value from tracex_ret_t otherwise.
 */
tracex_ret_t TRACEX_getHeader(struct tracex_handler *handler, struct tracex_header **header);
#endif