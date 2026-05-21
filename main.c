#include "trace_parser.h"
#include "trace_parser_debug.h"
#include "trace_parser_errno.h"
#include <stdint.h>
#include <stdio.h>


uint8_t *file_name = "test_dump.trx";
int main()
{
    int status;
    struct trace_parser *parser;

    parser = NULL;
    status = trace_parser_open(file_name, &parser);
    
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
            
            
    }
    if (status == 0)
    {
        trace_parser_debug_print_header(parser->parsed_trace.header->parsed_header);
        trace_parser_debug_parser(parser);
        status = trace_parser_parse_data(parser);
        
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


        }
        if (status == 0)
            trace_parser_debug_print_objects(parser);
    }

    trace_parser_close(parser);
    
}
