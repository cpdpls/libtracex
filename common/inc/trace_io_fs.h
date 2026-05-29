#ifndef __TRACE_IO_FS_H__
#define __TRACE_IO_FS_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "trace_errno.h"


typedef TRACERet_t __init__(void);
/* Custom sections declaration on MacOS toolchain */
#ifdef __APPLE__

// extern int start_fs_init_section __asm("section$start$__DATA$__fs_init_section");
// extern int stop_fs_init_section  __asm("section$end$__DATA$__fs_init_section");


#define TRACE_IO_FS_DRIVER_INIT(fn) \
static TRACERet_t (*__trace_io_fs_init_##fn)(void) \
    __attribute__((used, section("__DATA,__fs_init"))) = (fn)

#define TRACE_IO_FS_DRIVER_EXIT(fn) \
    static void (*__trace_io_fs_exit_##fn)(void) \
    __attribute__((used, section("__DATA,__fs_init"))) = (fn)


/* Linux section definition */
#else
#define TRACE_IO_FS_DRIVER_INIT(fn) \
    static TRACERet_t (*__trace_io_fs_init_##fn)(void) \
    __attribute__((used, section("trace_io_fs_init"))) = (fn)

#define TRACE_IO_FS_DRIVER_EXIT(fn) \
    static void (*__trace_io_fs_exit_##fn)(void) \
    __attribute__((used, section("trace_io_fs_exit"))) = (fn)

#define trace_io_fs_register_init_driver(__init_driver) \
    TRACE_IO_FS_DRIVER_INIT(__init_driver)

#define trace_io_fs_register_exit_driver(__exit_driver) \
    TRACE_IO_FS_DRIVER_EXIT(__exit_driver)

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