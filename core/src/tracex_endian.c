#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "tracex/tracex_errno.h"
#include "tracex_endian.h"

enum endianess
{
    E_LITTLE,
    E_BIG,
};

static enum endianess sys_endian = 0;

void tracex_utils_detect_indianess(void)
{
    const int x = 1;

    /* This is a big endian system */
    if ((*(char*)&x) == 0)
    {
        sys_endian = E_BIG;
    }
    else
    {
        sys_endian = E_LITTLE;
    }
}
