#include <stdio.h>
#include "trace.h"
#include "trace_io_fs.h"
#include <stdint.h>

int main(void)
{
    long sz = &stop_fs_init - &start_fs_init;
    uint64_t *p;

    printf("Section size is %ld\n", sz);
    printf("%p -- %p\n", &stop_fs_init, &start_fs_init);
    printf("%p -- %p\n", stop_fs_init, start_fs_init);

    for (p = &start_fs_init; p < &stop_fs_init; p++)
    {
        if (p)
        {
            ((__init__*)*p)();
        
        }
    }
    
    return 0;
}