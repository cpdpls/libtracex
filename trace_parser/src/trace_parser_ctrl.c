#include "trace_parser_ctrl.h"
#include "trace_ctrl.h"
#include "trace_parser.h"
#include "trace_parser_errno.h"
#include <stdio.h>

static struct trace_control_header *alloc_trace_parser_ctrl(void);
static inline int trace_parser_ctrl_check_magic_number(struct trace_control_header *control_header);
static int trace_parser_read_header(FILE *file_ptr, struct trace_control_header *control_header);

int trace_parser_ctrl_header(FILE * file_ptr, struct trace_control_header **control_header)
{
    int status;
    
    if (control_header == NULL)
    {
        status = TRACE_PARSER_INVALID_PTR;
        goto status_return;
    }

    *control_header = alloc_trace_parser_ctrl();
    if (*control_header == NULL)
    {
        status = TRACE_PARSER_MEM_ERR;
        goto cleanup_header_error;
    }

    status = trace_parser_read_header(file_ptr, *control_header);
    if (status != 0)
    {
        goto cleanup_header_error;
    }
    
    status = trace_parser_ctrl_check_magic_number(*control_header);
    if (status != 0)
    {
        goto cleanup_header_error;
    }
    
    return 0;

cleanup_header_error:
    trace_parser_ctrl_destroy(*control_header);

status_return:
    return status;
}
void trace_parser_ctrl_destroy(struct trace_control_header *control_header)
{
    if (control_header != NULL)
    {
        free(control_header);
    }

}

static struct trace_control_header *alloc_trace_parser_ctrl(void)
{
    struct trace_control_header *control_header;
    
    control_header = (void*)malloc(sizeof(struct trace_control_header));
    if (control_header == NULL)
    {
        return NULL;
    }

    return control_header;
}

static inline int trace_parser_ctrl_check_magic_number(struct trace_control_header *control_header)
{
    int magic_status;

    if (control_header->header_id != TRACE_CTRL_MAGIC_NUMBER_ID_LE &&
        control_header->header_id != TRACE_CTRL_MAGIC_NUMBER_ID_BE)
    {
        magic_status =  TRACE_PARSER_INVALID_MAGIC_NUMBER;
    }
    else
    {
        magic_status = 0;
    }
    return magic_status;
}
static int trace_parser_read_header(FILE *file_ptr, struct trace_control_header *control_header)
{
    int bytes_read;

    bytes_read = fread((void*)control_header, 1, sizeof(struct trace_control_header), file_ptr);

    if (bytes_read != sizeof(struct trace_control_header))
    {
        return TRACE_PARSER_FILE_OP_ERROR;
    }
    
    return 0;

}
