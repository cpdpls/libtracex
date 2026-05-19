#include "trace_parser.h"
#include "trace_ctrl.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int trace_parser_open(uint8_t *trace_path, struct trace_parser **parser_ptr)
{
    FILE *file_ptr;
    int file_size;
    int status;
    
    file_ptr = NULL;

    /* Sanitize parameters */
    if (trace_path == NULL || parser_ptr == NULL)
    {
        status = TRACE_PARSER_INVALID_PTR;
        goto status_return;
    }
    
    *parser_ptr = (struct trace_parser*)malloc(sizeof(struct trace_parser));
    if (*parser_ptr == NULL)
    {
        status = TRACE_PARSER_MEM_ERR;
        goto status_return;
    }
    
    (*parser_ptr)->trace_raw_buffer = NULL;

    /* Try to open the trace file */
    (*parser_ptr)->trace_file = fopen((const char*)trace_path, "rb");
    if ((*parser_ptr)->trace_file == NULL)
    {
        status = TRACE_PARSER_TRACE_FILE_NOT_FOUND;
        goto error_cleanup_all;
    }
    
    /* Go to the end of the file in order to tell the size */
    if (fseek((*parser_ptr)->trace_file , 0, SEEK_END) != 0)
    {
        status = TRACE_PARSER_FILE_OP_ERROR;
        goto error_cleanup_all;
    }
    
    /* Get the actual size of the file */
    (*parser_ptr)->trace_size = ftell(file_ptr);
    if ((*parser_ptr)->trace_size  == -1)
    {
        status = TRACE_PARSER_FILE_OP_ERROR;
        goto error_cleanup_all;
    }
    
    /* Rewind the file back to it's position */
    if (fseek((*parser_ptr)->trace_file, 0, SEEK_SET) != 0)
    {
        status = TRACE_PARSER_FILE_OP_ERROR;
        goto error_cleanup_all;
    }
    
    /* Check for an empty trace file */
    if ((*parser_ptr)->trace_size == 0)
    {
        status = TRACE_PARSER_EMPTY_TRACE_FILE;
        goto error_cleanup_all;
    }
    
    /* Check if we can at least parse the header part of the trace file */
    if ((*parser_ptr)->trace_size < sizeof(struct trace_control_header))
    {
        status = TRACE_PARSER_INVALID_TRACE_SIZE;
        goto error_cleanup_all;
    }
    
    /* Yes, don't trust the user :) */
    if ((*parser_ptr)->trace_size > SIZE_MAX)
    {
        status = TRACE_PARSER_FILE_TOO_BIG;
        goto error_cleanup_all;
    }
    
    (*parser_ptr)->trace_raw_buffer = (void*)malloc((*parser_ptr)->trace_size);
    if ((*parser_ptr)->trace_raw_buffer == NULL)
    {
        status = TRACE_PARSER_MEM_ERR;
        goto error_cleanup_all;
    }
    
    /* Read first the header to check for correct file type */
    if (fread((*parser_ptr)->trace_raw_buffer,
              1,
              sizeof(struct trace_control_header),
              (*parser_ptr)->trace_file) != sizeof(struct trace_control_header))
    {
        /* We didn't manage to read the header */
        status = TRACE_PARSER_FILE_OP_ERROR;
        goto error_cleanup_all;

    }
    if (fread((*parser_ptr)->trace_raw_buffer,
              1,
              (*parser_ptr)->trace_size,
              (*parser_ptr)->trace_file) != (*parser_ptr)->trace_size)
    {
        /* We didn't manage to read the whole file */
        status = TRACE_PARSER_FILE_OP_ERROR;
        goto error_cleanup_all;
    }
    

    status = 0;


error_cleanup_all:
    trace_parser_destroy(*parser_ptr);
    
status_return:
    return status;

}
void trace_parser_destroy(struct trace_parser *trace)
{
    if (trace != NULL)
    {
        if (trace->trace_file != NULL)
        {
            fclose(trace->trace_file);

        }
        if (trace->trace_raw_buffer!= NULL)
        {
            free(trace->trace_raw_buffer);
        }
        free(trace);
    }

    /*TODO: Call the destructor for the header */


}



