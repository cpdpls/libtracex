#include "tracex/tracex_errno.h"


static const char  *tracex_errno_strings[] =
{
    "TRACEX SUCESS",
    "TRACEX ALLOCATION ERROR",
    "TRACEX HAS NOT BEEN INITIALIZED. PLEASE CALL \"TRACEX_INIT\" FIRST",
    "TRACEX INITIALIZATION FAILURE",
    "TRACEX BAD INPUT POINTER",
    "TRACEX FAILURE TO REGISTER CALLBACKS",
    "TRACEX HEADER IS INVALID",
    "TRACEX NEED MORE DATA",
    "TRACEX LENGTH IS NULL",
    "TRACEX BAD OFFSET START IN HEADER PARSER",
    "TRACEX BAD OFFSET START IN OBJECT PARSER",
    "TRACEX INVALID OBJECT REGISTRY ADDRESSES",
    "TRACEX INVALID OBJECT",
    "TRACEX OBJECT IS DUPLICATED",
    "TRACEX OBJECT ITERATOR END",
    "TRACEX OBJECT ITERATOR IS INVALID OR CORRUPTED",
    "TRACEX TRACE EVENT BUFFER IS INVALID",
    "TRACEX EVENT IS INVALID",
    "TRACEX UNKNOWN ERROR",
};

const char *TRACEX_strerror(tracex_ret_t tracex_error)
{

    const char *error_string;

    /* Check for an unknown tracex error */
    if ((unsigned)tracex_error >= TRACEX_ERRNO_COUNT) {
        error_string = tracex_errno_strings[TRACEX_ERRNO_COUNT - 1];
    }

    /* Otherwise return the actual error string */
    else
    {
        error_string = tracex_errno_strings[tracex_error];
    }

    return error_string;


}