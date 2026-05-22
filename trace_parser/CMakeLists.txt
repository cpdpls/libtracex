add_library(traceparser STATIC)

target_sources( traceparser
                PRIVATE
                ${CMAKE_CURRENT_LIST_DIR}/src/trace_parser_debug.c
                ${CMAKE_CURRENT_LIST_DIR}/src/trace_parser_ctrl.c
                ${CMAKE_CURRENT_LIST_DIR}/src/trace_parser.c
                ${CMAKE_CURRENT_LIST_DIR}/src/trace_parser_object.c
                ${CMAKE_CURRENT_LIST_DIR}/src/trace_parser_event.c

)
target_include_directories(traceparser PUBLIC ${CMAKE_CURRENT_LIST_DIR}/inc)
