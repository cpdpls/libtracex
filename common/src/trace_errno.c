#include "trace_errno.h"


char *trace_errno_strings[] = {
    "TRACE SUCCESS",
    "TRACE ALLOCATION FAILURE",
    "TRACE BAD INPUT POINTER",
    "TRACE INVALID REGISTERED FS DRIVER",
    "TRACE NO EXISTING BACKEND REGISTERED",
    "TRACE FILE I/O ERROR",
    "TRACE NOT A TRACEX FILE",
    "TRACE FILE NOT FOUND",
    "TRACE FILE EMPTY",
    "TRACE UNKNOWN RETURN VAL",
};

char *tracestrerror(TRACERet_t trace_error)
{
    char *err_string;

    /* Check for an unknown error code */
    if(trace_error >= TRACE_LAST)
        err_string = trace_errno_strings[TRACE_LAST];
    /* Otherwise, assign the correct error code string */
    else
        err_string = trace_errno_strings[trace_error];

        return err_string;
}