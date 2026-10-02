#include "tracex/tracex_labels_errno.h"


const char  *tracex_labels_errno_strings[] =
{
    "TRACEX LABELS SUCESS",
    "TRACEX LABELS ALLOCATION ERROR",
    "TRACEX BAD INPUT POINTER",
    "TRACEX LABELS JSON READING FAILURE",
    "TRACEX LABELS JSON FILE NOT FOUND",
    "TRACEX LABELS JSON FILE EMPTY",
    "TRACEX LABELS JSON ALLOCATION FAILURE",
    "TRACEX LABELS JSON PARSING FAILURE",
    "TRACEX UNKNOWN ERROR",
};

const char *tracex_labels_strerror(tracex_labels_ret_t tracex_error)
{

    const char *error_string;

    /* Check for an unknown tracex error */
    if (tracex_error >= TRACEX_LABELS_ERRNO_COUNT) {
        error_string = tracex_labels_errno_strings[TRACEX_LABELS_ERRNO_COUNT - 1];
    }

    /* Otherwise return the actual error string */
    else
    {
        error_string = tracex_labels_errno_strings[tracex_error];
    }

    return error_string;


}