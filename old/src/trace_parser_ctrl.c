#include "trace_parser_ctrl.h"
#include "trace_ctrl.h"
#include "trace_parser.h"
#include "trace_parser_errno.h"
#include <stddef.h>
#include <stdio.h>

static struct trace_control_header *alloc_trace_parser_ctrl(void);
static inline int trace_parser_ctrl_check_magic_number(struct trace_control_header *header);
static int trace_parser_read_header(FILE *file_ptr, struct trace_control_header *header);

int trace_parser_ctrl_header(FILE * file_ptr, struct trace_control_header **header)
{
    int status;
    
    if (header == NULL)
    {
        status = TRACE_PARSER_INVALID_PTR;
        goto status_return;
    }
    
    *header = alloc_trace_parser_ctrl();
    status = trace_parser_read_header(file_ptr, *header);
    if (status != 0)
    {
        goto cleanup_header_error;
    }
    
    status = trace_parser_ctrl_check_magic_number(*header);
    if (status != 0)
    {
        goto cleanup_header_error;
    }
    
    return 0;

cleanup_header_error:
    trace_parser_ctrl_destroy(header);

status_return:
    return status;
}
void trace_parser_ctrl_destroy(struct trace_control_header **header)
{
    if (header != NULL)
    {
        if (*header != NULL)
        {
            if ((*header)->raw_buffer != NULL)
            {
                free((*header)->raw_buffer);
                (*header)->raw_buffer = NULL;
            }

            (*header)->parsed_header = NULL;
            free(*header);
            *header = NULL;

        }
        
    }

}

static struct trace_control_header *alloc_trace_parser_ctrl(void)
{
    struct trace_control_header *tmp_header;
    
    tmp_header = (void*)malloc(sizeof(struct trace_control_header));
    if (tmp_header == NULL)
    {
        return NULL;
    }
    tmp_header->raw_buffer = (void*)malloc(sizeof(struct trace_parsed_control_header));

    if (tmp_header->raw_buffer == NULL)
    {
        return NULL;
    }
    return tmp_header;
}

static inline int trace_parser_ctrl_check_magic_number(struct trace_control_header *header)
{
    int magic_status;

    if (header->parsed_header->header_id != TRACE_CTRL_MAGIC_NUMBER_ID_LE &&
        header->parsed_header->header_id != TRACE_CTRL_MAGIC_NUMBER_ID_BE)
    {
        magic_status =  TRACE_PARSER_INVALID_MAGIC_NUMBER;
    }
    else
    {
        magic_status = 0;
    }
    return magic_status;
}
static int trace_parser_read_header(FILE *file_ptr, struct trace_control_header *header)
{
    size_t bytes_read;
    
    bytes_read = fread((void*)header->raw_buffer, 1, sizeof(struct trace_parsed_control_header), file_ptr);

    if (bytes_read != sizeof(struct trace_parsed_control_header))
    {
        return TRACE_PARSER_FILE_OP_ERROR;
    }
    
    /* Assign the parsed ptr to the raw buffer */
    header->parsed_header = (struct trace_parsed_control_header*)header->raw_buffer;
    header->raw_buffer_size = bytes_read;    
    return 0;

}
