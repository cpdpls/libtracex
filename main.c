#include "trace_parser.h"
#include "trace_parser_debug.h"
#include "trace_parser_errno.h"
#include <stdint.h>
#include <stdio.h>


uint8_t *file_name = "r15b_all_int.trx";
int main()
{
    int status;
    struct trace_parser *parser;

    parser = NULL;
    
    printf("Opening trace file ...\n");
    status = trace_parser_open(file_name, &parser);
    
    if (status != 0)
    {
        switch(status)
        {
            case TRACE_PARSER_INVALID_PTR:
                perror("Internal Invalid Pointer !\n");
                break;

            case TRACE_PARSER_MEM_ERR:
                perror("Internal Memory error !\n");
                break;

            case TRACE_PARSER_FILE_OP_ERROR:
                perror("Internal file operation error !\n");
                break;

            case TRACE_PARSER_TRACE_FILE_NOT_FOUND:
                fprintf(stderr, "Trace file \"%s\" not found !\n", file_name);
                break;

            case TRACE_PARSER_EMPTY_TRACE_FILE:
                fprintf(stderr, "Trace file \"%s\" is empty !\n", file_name);
                break;

            case TRACE_PARSER_INVALID_TRACE_SIZE:
                fprintf(stderr, "Trace file \"%s\" has an invalid size !\n", file_name);
                break;

            case TRACE_PARSER_INVALID_MAGIC_NUMBER:
                fprintf(stderr, "Trace file \"%s\" is an invalid traceX dump !\n", file_name);
                break;

            case TRACE_PARSER_CORRUPTED_CTRL_HEADER:
                fprintf(stderr, "Trace file \"%s\" has a corrupted header !\n", file_name);
                break;

            default:
                /* Do nothing */
                break;


                trace_parser_close(parser);
                return status;

        }

    }

    /* Parsing data ... */
    printf("Open success !\n");
    printf("Parsing data ...\n");
    status = trace_parser_parse_data(parser);
    
    if (status != 0)
    {

        switch(status)
        {
            case TRACE_PARSER_INVALID_PTR:
                perror("Internal Invalid Pointer !\n");
                break;

            case TRACE_PARSER_MEM_ERR:
                perror("Internal Memory error !\n");
                break;

            case TRACE_PARSER_FILE_OP_ERROR:
                perror("Internal file operation error !\n");
                break;

            default:
                /* Do nothing */
                break;


        }
        trace_parser_close(parser);
        return status;
    }
    else
    {
        printf("Parsing done !\n");
    }

    
    trace_parser_debug_print_header(parser->parsed_trace.header->parsed_header);
    trace_parser_debug_print_used_objects(parser);
    trace_parser_printf_summary(parser);
    trace_parser_close(parser);

    
}
