#include "trace_io_dev.h"
#include "trace_io.h"

struct trace_io_dev *create_io_dev(void)
{
    struct trace_io_dev *tmp_dev;

    tmp_dev = (struct trace_io_dev *)malloc(sizeof(struct trace_io_dev));

    if(tmp_dev == NULL)
    {
        return NULL;
    }
    return tmp_dev;
}
void destroy_io_dev(struct trace_io_dev **dev)
{
    if (dev != NULL)
    {
        if (*dev != NULL)
        {
            free(*dev);
            *dev = NULL;
        }
    }
}

TRACERet_t trace_io_from_tcp(struct trace_io_dev **io_dev, tcp_info *tcp_info)
{
    return TRACE_SUCCESS;

}

TRACERet_t trace_io_read(struct trace_io_dev *io_dev, size_t len, size_t *bytes_read, void *buf)
{
    return TRACE_SUCCESS;

}

TRACERet_t trace_io_peek(struct trace_io_dev *io_dev, size_t len, size_t *bytes_read, void *buf)
{
    return TRACE_SUCCESS;
}

TRACERet_t trace_io_tell(struct trace_io_dev *io_dev, size_t *offset)
{
    return TRACE_SUCCESS;
}

TRACERet_t trace_io_seek(struct trace_io_dev *io_dev, size_t offset , enum io_whence whence)
{
    return TRACE_SUCCESS;
}

void trace_io_destroy_dev(struct trace_io_dev **io_dev)
{
    if (io_dev != NULL)
    {
        if (*io_dev != NULL)
        {
            if ((*io_dev)->destroy != NULL)
            {
                (*io_dev)->destroy(io_dev);
            }
        }

    }
}