#include <stddef.h>
#include <stdio.h>

#include "trace_parser_debug.h"
#include "trace_ctrl.h"
#include "trace_events.h"
#include "trace_parser.h"
#include "trace_objects_registry.h"
#include "trace_parser_object.h"
#include "stdlib.h"

static void print_highest_isr(struct trace_parser *parser);
static void print_total_ticks(struct trace_parser *parser);
void trace_parser_debug_print_header(struct trace_parsed_control_header *control_header)
{
    if (control_header == NULL)
    {
        perror("TRACE_HEADER_DEBUG : Invalid header pointer !");
    }
    else
    {
        printf("----TRACE_HEADER_DEBUG----\n\n");
        printf("Header ID --> %.*s\n", sizeof(control_header->header_id),&control_header->header_id);
        printf("Header timer_valid_mask --> 0x%08x\n", control_header->header_timer_valid_mask);
        printf("Header trace base address --> 0x%08x\n", control_header->header_trace_base_addr);
        printf("Header registry start pointer --> 0x%08x\n", control_header->header_obj_registry_start_ptr);
        printf("Header reserved 1 --> %hu\n", control_header->header_res1);
        printf("Header registry name size --> %hu\n", control_header->header_obj_registry_name_size);
        printf("Header registry end pointer --> 0x%08x\n", control_header->header_obj_registry_end_ptr);
        printf("Header trace start pointer --> 0x%08x\n", control_header->header_buffer_start_ptr);
        printf("Header trace end pointer --> 0x%08x\n", control_header->header_buffer_end_ptr);
        printf("Header trace current pointer --> 0x%08x\n", control_header->header_buffer_current_ptr);
        printf("Header reserved 2 --> %d\n", control_header->header_res2);
        printf("Header reserved 3 --> %d\n", control_header->header_res3);
        printf("Header reserved 4 --> %d\n\n\n", control_header->header_res4);
    }
}

void trace_parser_debug_print_object(struct trace_parser *parser, int obj_index)
{
    struct trace_object_entry *object;
    if (parser == NULL)
        return;
    if (obj_index > parser->parsed_trace.objects_registry->parsed_registry->objects_count)
        return;


    object = parser->parsed_trace.objects_registry->parsed_registry->all_objects[obj_index];
    
    printf("\n----- %.*s -----\n",parser->parsed_trace.header->parsed_header->header_obj_registry_name_size, &object->obj_name );
    printf("Type : %s\n", trace_parse_object_get_type(obj_index, parser->parsed_trace.objects_registry));
    printf("Used : %s\n", object->obj_available ? "NO" : "YES");

    if (object->obj_type == OBJ_THREAD)
    {
        printf("Thread Priority : %hd\n", (short)object->obj_res2);
    }
    printf("Pointer : 0x%08x\n", object->obj_ptr);
    printf("param 1 : %d\n", object->obj_param_1);
    printf("param 2 : %d\n", object->obj_param_2);


}

void trace_parser_debug_parser(struct trace_parser *parser)
{
    if (parser != NULL)
    {
        if (parser->endianess == TRACE_PARSER_LE)
        {
            printf("Trace file is little endian !");
        }
        else
    {
            printf("Trace file is big endian !");

        }
    }
}
void trace_parser_debug_print_all_objects(struct trace_parser *parser)
{
    size_t i;
    if (parser == NULL)
    {
        printf("DEBUG : BAD PTR !\n");
        return;
    }


    for (i = 0; i < parser->parsed_trace.objects_registry->parsed_registry->objects_count; i++)
    {
        trace_parser_debug_print_object(parser, i);
    }



}
void trace_parser_debug_print_used_objects(struct trace_parser *parser)
{
    size_t i;
    struct trace_object_entry **object;
    if (parser == NULL)
    {
        printf("DEBUG : BAD PTR !\n");
        return;
    }


    for (i = 0; i < parser->parsed_trace.objects_registry->parsed_registry->objects_count; i++)
    {
        if (!parser->parsed_trace.objects_registry->parsed_registry->all_objects[i]->obj_available)
            trace_parser_debug_print_object(parser, i);
    }

}
void trace_parser_printf_summary(struct trace_parser *parser)
{
    if (parser == NULL)
    {
        printf("DEBUG : PAD PTR !\n");
        return;
    }
    
    printf("\n------ SUMMARY ------\n");
    printf("Number of objects parsed (%d)\n", parser->parsed_trace.objects_registry->parsed_registry->objects_count);
    printf("Actual objects in use (%d)\n", parser->parsed_trace.objects_registry->parsed_registry->objects_in_use);
    printf("Number of events parsed (%d)\n", parser->parsed_trace.events_regitry->parsed_registry->events_count);
    print_total_ticks(parser);
    print_highest_isr(parser);
}
void trace_parser_print_list_used_object(struct trace_parser *parser)
{
    size_t i;
    struct trace_object_entry *obj;

    for (i = 0; i < parser->parsed_trace.objects_registry->parsed_registry->objects_in_use; i++)
    {
        obj = parser->parsed_trace.objects_registry->parsed_registry->used_objects[i];
        printf("%d. %.*s\n", i,parser->parsed_trace.header->parsed_header->header_obj_registry_name_size, &obj->obj_name);
    }

}
static void print_highest_isr(struct trace_parser *parser)
{
    size_t i;
    size_t isr_enter_count;
    size_t highest_isr_number;
    size_t *isr_counter;
    struct trace_event_entry *event;
    size_t highest_isr_enter;
    size_t isr_highest_index;

    
    isr_enter_count = 0;
    highest_isr_number = 0;
    for (i = 0; i < parser->parsed_trace.events_regitry->parsed_registry->events_count; i++)
    {
        event = parser->parsed_trace.events_regitry->parsed_registry->events[i];
        if (event->event_id == ISR_ENTER)
        {
            if (event->info_2 > highest_isr_number)
            {
                highest_isr_number = event->info_2;
            }
            isr_enter_count++;
        }
    }

    isr_counter = malloc(sizeof(size_t) * (highest_isr_number + 1));
    
    if (isr_counter == NULL)
    {
        printf("DEBUG : Internal mem error !\n");
        return;
    }
    
    for (i = 0; i < highest_isr_number + 1; i++)
    {
        isr_counter[i] = 0;
    }
    
    highest_isr_enter = 0;
    for (i = 0; i < parser->parsed_trace.events_regitry->parsed_registry->events_count; i++)
    {
        event = parser->parsed_trace.events_regitry->parsed_registry->events[i];

        if (event->event_id == ISR_ENTER)
        {
            isr_counter[event->info_2]++;

            if (isr_counter[event->info_2] > highest_isr_enter)
            {
                isr_highest_index = event->info_2;
                highest_isr_enter = isr_counter[event->info_2];

            }
        }
    }
    //
    printf("Highest isr number : %ld\n", highest_isr_number);
    printf("---- Total occurences per ISR ----\n\n");
    for (i = 0; i < highest_isr_number + 1; i++)
    {
        printf("ISR %ld --> %ld Interrupts\n", i, isr_counter[i]);
    }
    //
    printf("Number of ISR's : %lu\n", isr_enter_count);
    printf("Highest ISR number entered : %lu with %lu interrupts\n", isr_highest_index, isr_counter[isr_highest_index]);
    
    free(isr_counter);
}
static void print_total_ticks(struct trace_parser *parser)
{
    size_t total_ticks;
    size_t total_event;

    total_event = parser->parsed_trace.events_regitry->parsed_registry->events_count;

    total_ticks =   parser->parsed_trace.events_regitry->parsed_registry->events[0]->time_stamp -
                    parser->parsed_trace.events_regitry->parsed_registry->events[total_event-1]->time_stamp;

    printf("Total ticks inside the trace : %lu\n", total_ticks);

}
