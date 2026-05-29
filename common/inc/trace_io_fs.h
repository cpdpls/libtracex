#ifndef __TRACE_IO_FS_H__
#define __TRACE_IO_FS_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "trace_errno.h"


typedef TRACERet_t __init__(void);
/* Custom sections declaration on MacOS platform */
#ifdef __APPLE__

/* Actual pointers used to call each registered fs driver */
extern uint8_t start_fs_init __asm("section$start$__DATA$__fs_init");
extern uint8_t stop_fs_init  __asm("section$end$__DATA$__fs_init");

#define TRACE_IO_FS_DRIVER_INIT(fn) \
static TRACERet_t (*__trace_io_fs_init_##fn)(void) \
    __attribute__((used, section("__DATA,__fs_init"))) = (fn)


/* Custom sections declaration on Linux platform */
#else
/* Actual pointers used to call each registered fs driver */
extern 

#define TRACE_IO_FS_DRIVER_INIT(fn) \
    static TRACERet_t (*__trace_io_fs_init_##fn)(void) \
    __attribute__((used, section(".fs_init"))) = (fn)


#define trace_io_fs_register_init_driver(__init_driver) \
    TRACE_IO_FS_DRIVER_INIT(__init_driver)

#endif


typedef struct trace_io_fs trace_io_fs;

struct trace_io_fs_ops
{
    TRACERet_t (*open)(trace_io_fs *, const char *path);
    TRACERet_t (*close)(trace_io_fs * fs);
    TRACERet_t (*read)(trace_io_fs * fs, size_t len, size_t *bytes_read, void *buf);
};

TRACERet_t trace_io_fs_register(const char *scheme, const struct trace_io_fs_ops *ops);

void trace_io_fs_init(void);
void trace_io_fs_unregister(const char *scheme);
void trace_io_fs_unregister_all(void);
#endif