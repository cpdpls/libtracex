#ifndef __TRACE_IO_H__
#define __TRACE_IO_H__

#include <stdlib.h>
#include <stdint.h>
#include "trace_errno.h"

/* Forward declaration of the internal dev structure */

typedef struct trace_io_dev trace_io_dev;

enum io_whence
{
    IO_WHENCE_SET,
    IO_WHENCE_CUR,
    IO_WHENCE_END

};

typedef struct tcp_info
{
    const char *host;
    uint16_t port;
    int timeout_sec;
}tcp_info;

/**
 * @brief Initializes the device whose source is a file 
 * 
 * @param io_dev Returned created device
 * @param path The path to the device source file
 * @return TRACERet_t function call result 
 */
TRACERet_t trace_io_from_file(struct trace_io_dev **io_dev, const char *path);

/**
 * @brief Initializes the device whose source is a tcp socket connection
 * 
 * @param io_dev Returned created device
 * @param tcp_info  Structure who needs to be filled in before calling this function
 *                  which contains the info needed in order to initialize the tcp socket
 * @return TRACERet_t Function call result
 */
TRACERet_t trace_io_from_tcp(struct trace_io_dev **io_dev, tcp_info *tcp_info);

TRACERet_t trace_io_read(struct trace_io_dev *io_dev, size_t len, size_t *bytes_read, void *buf);

TRACERet_t trace_io_peek(struct trace_io_dev *io_dev, size_t len, size_t *bytes_read, void *buf);

TRACERet_t trace_io_tell(struct trace_io_dev *io_dev, size_t *offset);

TRACERet_t trace_io_seek(struct trace_io_dev *io_dev, size_t offset , enum io_whence whence);

/**
 * @brief 
 * 
 *
 * @brief Destroy a previously created device
 * 
 * @param io_dev Pointer to the device about to be deleted
 */
void trace_io_destroy_dev(struct trace_io_dev **io_dev);

#endif