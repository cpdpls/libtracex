#include "trace_parser.h"
#include "trace_ctrl.h"
#include "trace_parser_ctrl.h"
#include "trace_parser_errno.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

static int open_trace_file(struct trace_parser *parser, uint8_t *trace_path);
static int check_trace_size(struct trace_parser *parser);
static inline int trace_parser_set_endianess(struct trace_parser *parser);
static struct trace_parser *alloc_parser(void);

static int open_trace_file(struct trace_parser *parser, uint8_t *trace_path)
{

    /* Try to open the trace file */
    parser->trace_file = fopen((const char*)trace_path, "rb");
    if (parser->trace_file == NULL)
    {
        return TRACE_PARSER_TRACE_FILE_NOT_FOUND;
    }
    
    /* Go to the end of the file in order to tell the size */
    if (fseek(parser->trace_file, 0, SEEK_END) != 0)
    {
        return TRACE_PARSER_FILE_OP_ERROR;
    }
    
    /* Get the actual size of the file */
    parser->trace_size = ftell(parser->trace_file);
    if (parser->trace_size == -1)
    {
        return TRACE_PARSER_FILE_OP_ERROR;
    }
    
    /* Rewind the file back to it's position */
    if (fseek(parser->trace_file, 0, SEEK_SET) != 0)
    {
        return TRACE_PARSER_FILE_OP_ERROR;
    }

    return 0;

}
static int check_trace_size(struct trace_parser *parser)
{
    /* Check for an empty trace file */
    if (parser->trace_size == 0)
    {
        return TRACE_PARSER_EMPTY_TRACE_FILE;
    }
    
    /* Check if we can at least parse the header part of the trace file */
    if (parser->trace_size < sizeof(struct trace_control_header))
    {
        return TRACE_PARSER_INVALID_TRACE_SIZE;
    }
    
    /* Yes, don't trust the user :) */
    if (parser->trace_size > SIZE_MAX)
    {
        return TRACE_PARSER_FILE_TOO_BIG;
    }

    return 0;

}
int trace_parser_open(uint8_t *trace_path, struct trace_parser **parser_ptr)
{
    struct trace_parser *temp_parser;
    int status;
    
    temp_parser = NULL;

    /* Sanitize parameters */
    if (trace_path == NULL || parser_ptr == NULL)
    {
        status = TRACE_PARSER_INVALID_PTR;
        goto status_return;
    }

    /* Alloc a temp parser used for checking */ 
    temp_parser = alloc_parser();
    if (temp_parser == NULL)
    {
        status = TRACE_PARSER_MEM_ERR;
        goto error_cleanup_all;
    }
    
    /* Try to open the trace file and read it's size */
    status = open_trace_file(temp_parser, trace_path);
    if (status != 0)
    {
        goto error_cleanup_all;
    }
    
    /* Check if the size of the file makes sense */
    status = check_trace_size(temp_parser);
    if (status != 0)
    {
        goto error_cleanup_all;
    }
    
    /* Parse the header file to check for the correct file type */
    status = trace_parser_ctrl_header(temp_parser->trace_file, &temp_parser->parsed_trace.control_header);
    if (status != 0)
    {
        goto error_cleanup_all;

    }

    temp_parser->trace_raw_buffer = (void*)malloc(temp_parser->trace_size);

    if (temp_parser->trace_raw_buffer == NULL)
    {
        status = TRACE_PARSER_MEM_ERR;
        goto error_cleanup_all;
    }
    
    /* Copy the just parsed header into the raw buffer */
    /* This is done to avoid having to do an expensive rewind call on the file desc */
    *(struct trace_control_header*)temp_parser->trace_raw_buffer = *temp_parser->parsed_trace.control_header;
    
    /* It's now safe to read the rest of the file into the buffer */
    if (fread(temp_parser->trace_raw_buffer + sizeof(struct trace_control_header),
              1,
              temp_parser->trace_size - sizeof(struct trace_control_header),
              temp_parser->trace_file) != temp_parser->trace_size - sizeof(struct trace_control_header))
    {
        /* We didn't manage to read the whole file */
        status = TRACE_PARSER_FILE_OP_ERROR;
        goto error_cleanup_all;
    }

    /* We finally set the endianess of the file */
    status = trace_parser_set_endianess(temp_parser);
    if (status != 0)
    {
        goto error_cleanup_all;
    }
    status = 0;
    /* Assign the called parser to the filled in temp parser */
    *parser_ptr = temp_parser;

    goto status_return;

error_cleanup_all:
    trace_parser_close(temp_parser);
    
status_return:
    return status;

}
void trace_parser_close(struct trace_parser *trace)
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
        trace_parser_ctrl_destroy(trace->parsed_trace.control_header);
        free(trace);
    }

}

static struct trace_parser *alloc_parser(void)
{
    struct trace_parser *temp_parser;

    temp_parser = (struct trace_parser*)malloc(sizeof(struct trace_parser));
    if (temp_parser == NULL)
        return NULL;

    temp_parser->trace_file = NULL;
    temp_parser->trace_raw_buffer = NULL;
    temp_parser->parsed_trace.control_header = NULL;

    return temp_parser;
}


int trace_parser_set_endianess(struct trace_parser *parser)
{
    if(parser->parsed_trace.control_header->header_id == TRACE_CTRL_MAGIC_NUMBER_ID_BE)
    {
        parser->endianess = TRACE_PARSER_BE;
        return 0;
    }
    if (parser->parsed_trace.control_header->header_id == TRACE_CTRL_MAGIC_NUMBER_ID_LE)
    {
        parser->endianess = TRACE_PARSER_LE;
        return 0;
    }

    else
        return TRACE_PARSER_INVALID_MAGIC_NUMBER;

}
