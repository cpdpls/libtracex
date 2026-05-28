#ifndef __TRACE_ERRNO_H__
#define __TRACE_ERRNO_H__

typedef enum {
    TRACE_SUCCESS,
    TRACE_ALLOC_FAIL,
    TRACE_BAD_INPUT_PTR,
    TRACE_NO_EXISTING_BACKEND,
    TRACE_FILE_IO_ERROR,
    TRACE_NOT_TRX_FILE,
    TRACE_TRX_NOT_FOUND,
    TRACE_EMPTY_TRX,
    TRACE_LAST,
}TRACERet_t;

/**
 * @brief Returns the associated String error from a defined array
 *
 * @param trace_error The error to return the string
 * @return char* The returned error string
 */
char *tracestrerror(TRACERet_t trace_error);

#endif