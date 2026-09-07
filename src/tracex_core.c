#include <stdlib.h>
#include <string.h>

#include "tracex_errno.h"
#include "tracex_core.h"
#include "tracex_event_int.h"
#include "tracex_obj_int.h"

static struct tracex_list parser_list;

static TRACEX_Ret_t tracex_check_parser_valid(struct TRACEX_handler_t *parser);

struct TRACEX_handler_t *TRACEX_createParser(void)
{
    struct TRACEX_handler_t *tmp_parser;

    tmp_parser = (struct TRACEX_handler_t*)malloc(sizeof(struct TRACEX_handler_t));

    if (tmp_parser == NULL)
    {
        return NULL;
    }

    memset(tmp_parser, 0, sizeof(struct TRACEX_handler_t));

    if (parser_list.next == NULL || parser_list.prev == NULL)
    {
        tracex_list_init(&parser_list);
    }

    tracex_list_init(&tmp_parser->parsed_orig_dump.obj_list);
    tracex_list_init(&tmp_parser->parsed_orig_dump.event_list);
    tracex_list_insert(&tmp_parser->node, &parser_list);

    return tmp_parser;
}

void TRACEX_destroyParser(struct TRACEX_handler_t **parser)
{
    struct tracex_event_entry_t *iter_event;
    if (parser != NULL)
    {
        if (*parser != NULL)
        {
            tracex_destroy_event_list(&(*parser)->parsed_orig_dump.event_list);
            tracex_destroy_object_list(&(*parser)->parsed_orig_dump.obj_list);
            tracex_list_delete(&(*parser)->node);
            memset((*parser), 0, sizeof(struct TRACEX_handler_t));
            free(*parser);
            *parser = NULL;
        }
    }

}

TRACEX_Ret_t TRACEX_parse(struct TRACEX_handler_t *parser, void *buffer, size_t buffer_length)
{
    TRACEX_Ret_t status;
    uint64_t consumed;
    struct tracex_hdr_entry_t *hdr_ptr;

    if (parser == NULL || buffer == NULL)
    {
        return TRACEX_BAD_INPUT_PTR;
    }

    if (buffer_length == 0)
    {
        return TRACEX_NULL_LENGTH;
    }

    hdr_ptr = &parser->parsed_orig_dump.header;

    if (hdr_ptr->is_header_processed && !hdr_ptr->is_header_valid)
    {
        return TRACEX_HEADER_NOT_VALID;
    }

    if (parser->state == E_HEADER_PHASE)
    {
        status = tracex_header_add_data(&parser->parsed_orig_dump.header, buffer, buffer_length, &consumed);

        if (status == TRACEX_SUCCESS)
        {
            parser->state = E_OBJECT_PHASE;
        }
        if (status == TRACEX_NEED_MORE)
        {

        }
        else
        {
            parser->header_valid = 0;

        }
    }

    parser->dump_size += buffer_length;

    return status;
}


TRACEX_Ret_t TRACEX_getTimerMask(struct TRACEX_handler_t *parser, uint32_t *mask)
{
    struct TRACEX_handler_t *iter;
    
    if (parser == NULL || mask == NULL)
    {
        return TRACEX_BAD_INPUT_PTR;
    }
    
    if (tracex_check_parser_valid(parser) != TRACEX_SUCCESS)
    {
        return TRACEX_INVALID_PARSER;
    }
    
    if (!parser->header_valid)
    {
        return TRACEX_HEADER_NOT_VALID;
    }

    return tracex_header_get_mask(&parser->parsed_orig_dump.header, mask);
    
}

static TRACEX_Ret_t tracex_check_parser_valid(struct TRACEX_handler_t *parser)
{
    struct TRACEX_handler_t *iter;

    tracex_list_for_each_entry(iter, &parser_list, node)
    {
        if (iter == parser)
        {
            return TRACEX_SUCCESS;
        }
    }

    return TRACEX_INVALID_PARSER;
    
}