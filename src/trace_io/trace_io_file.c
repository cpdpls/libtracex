#include <stdio.h>
#include <stdint.h>

#include "trace_io.h"
#include "trace_io_dev.h"


static void destroy_io_file_dev(struct trace_io_dev **dev);
static TRACERet_t trace_io_file_read(size_t len, size_t *bytes_read, void *buf);


TRACERet_t trace_io_from_file(struct trace_io_dev **io_dev, const char *path)
{
    TRACERet_t status;

    if (io_dev == NULL || path == NULL)
    {
        status = TRACE_BAD_INPUT_PTR;
        goto return_status;
    }

    /* Allocate a dummy device */
    *io_dev = create_io_dev();
    if (*io_dev == NULL)
    {
        status = TRACE_ALLOC_FAIL;
        goto return_status;
    }
    (*io_dev)->file_dev.path = NULL;
    (*io_dev)->file_dev.descriptor = NULL;

    /* Try to open the file for errors */
    (*io_dev)->file_dev.descriptor = fopen(path, "rb");
    if ((*io_dev)->file_dev.descriptor == NULL)
    {
        status = TRACE_TRX_NOT_FOUND;
        goto cleanup_failure;
    }

    /* Now, try to get the file size and check for an empty file */
    if (fseek((*io_dev)->file_dev.descriptor, 0, SEEK_END) != 0)
    {
        status = TRACE_FILE_IO_ERROR;
        goto cleanup_failure;
    }

    (*io_dev)->file_dev.file_size = ftell((*io_dev)->file_dev.descriptor);

    /* Check for an internal File operation error */
    if ((*io_dev)->file_dev.file_size == -1)
    {
        status = TRACE_FILE_IO_ERROR;
        goto cleanup_failure;
    }
    /* Check for an empty file */
    if ((*io_dev)->file_dev.file_size == 0)
    {
        status = TRACE_EMPTY_TRX;
        goto cleanup_failure;
    }

    /* Let's rewind back the file to the beginning */
    if (fseek((*io_dev)->file_dev.descriptor, 0, SEEK_SET) != 0)
    {
        status = TRACE_FILE_IO_ERROR;
        goto cleanup_failure;
    }

    /* Assign the device type before returning from this function call */
    (*io_dev)->dev_type = DEV_FILE;
    (*io_dev)->destroy = destroy_io_file_dev;

    status = TRACE_SUCCESS;
    goto return_status;

cleanup_failure:
    destroy_io_dev(io_dev);

return_status:
    return status;
}

/**
 * @brief Main function used for read operation on a file
 * 
 * @param len Bytes to read
 * @param bytes_read Actual bytes read
 * @param buf Destination buffer where data is placed
 * @return TRACERet_t Function call status
 */
static TRACERet_t trace_io_file_read(size_t len, size_t *bytes_read, void *buf)
{
    int read_sts;

    if (bytes_read == NULL || buf == NULL || 0 == len)
    {
        return TRACE_BAD_INPUT_PTR;
    }

    return TRACE_SUCCESS;

}

/**
 * @brief destroy io file dev object
 * 
 * @param dev Pointer to the device to be destroyed
 */
static void destroy_io_file_dev(struct trace_io_dev **dev)
{
    if (dev != NULL)
    {
        if (*dev != NULL)
        {
            if ((*dev)->file_dev.path != NULL)
            {
                free((*dev)->file_dev.path);
                (*dev)->file_dev.path = NULL;
            }
            if ((*dev)->file_dev.descriptor != NULL)
            {
                fclose((*dev)->file_dev.descriptor);
                (*dev)->file_dev.descriptor = NULL;
            }
            destroy_io_dev(dev);
        }
    }
}