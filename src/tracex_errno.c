#include "tracex_errno.h"


const char  *tracex_errno_strings[] =
{
    "TRACEX SUCESS",
    "TRACEX ALLOCATION ERROR",
    "TRACEX HAS NOT BEEN INITIALIZED. PLEASE CALL \"TRACEX_INIT\" FIRST",
    "TRACEX INITIALIZATION FAILURE",
    "TRACEX BAD INPUT POINTER",
    "TRACEX INVALID PROVIDED HANDLER",
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
    "TRACEX UNKNOWN ERROR",
};

const char *TRACEX_strerror(TRACEX_Ret_t tracex_error)
{

    const char *error_string;

    /* Check for an unknown tracex error */
    if (tracex_error >= TRACEX_NUMBER_OF_ERRORS) {
        error_string = tracex_errno_strings[TRACEX_NUMBER_OF_ERRORS - 1];
    }

    /* Otherwise return the actual error string */
    else
    {
        error_string = tracex_errno_strings[tracex_error];
    }

    return error_string;


}