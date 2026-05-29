#include <stdio.h>
#include "trace.h"
#include "trace_io_fs.h"
#include <stdint.h>

extern uintptr_t start_fs __asm("section$start$__DATA$__fs_init");
extern uintptr_t stop_fs  __asm("section$end$__DATA$__fs_init");

int main(void)
{
    long sz = &stop_fs - &start_fs;
    uintptr_t *p;

    printf("Section size is %ld\n", sz);
    printf("%p -- %p\n", &stop_fs, &start_fs);
    printf("%p -- %p\n", stop_fs, start_fs);

    for (p = &start_fs; p < &stop_fs; p++)
    {
        if (p)
        {
            ((__init__*)*p)();
        
        }
    }
    
    return 0;
}