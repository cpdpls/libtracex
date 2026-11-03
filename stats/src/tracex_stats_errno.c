#include "tracex_stats/tracex_stats_errno.h"


static const char  *tracex_stats_errno_strings[] =
{
    "TRACEX STATS SUCESS",
    "TRACEX STATS BAD INPUT POINTER",
    "TRACEX STATS ALLOCATION FAILURE",
    "TRACEX STATS INVALID LUA HANDLER",
    "TRACEX STATS LUA EXECUTION ERROR",
    "TRACEX STATS SCRIPT IS MISSING A FUNCTION",
    "TRACEX STATS SCRIPT IS MISSING A TABLE",
    "TRACEX STATS UNKNOWN ERROR",
};

const char *tracex_stats_strerror(tracex_stats_ret_t tracex_stats_error)
{

    const char *error_string;

    /* Check for an unknown tracex stats error */
    if ((unsigned)tracex_stats_error >= TRACEX_STATS_ERRNO_COUNT) {
        error_string = tracex_stats_errno_strings[TRACEX_STATS_ERRNO_COUNT - 1];
    }

    /* Otherwise return the actual error string */
    else
    {
        error_string = tracex_stats_errno_strings[tracex_stats_error];
    }

    return error_string;


}