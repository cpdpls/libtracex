add_library(libtraceparser STATIC)

target_sources( libtraceparser
                PRIVATE
                ${CMAKE_CURRENT_LIST_DIR}/src/trace_parser_debug.c
                ${CMAKE_CURRENT_LIST_DIR}/src/trace_parser_ctrl.c
                ${CMAKE_CURRENT_LIST_DIR}/src/trace_parser.c

)
target_include_directories(libtraceparser PUBLIC ${CMAKE_CURRENT_LIST_DIR}/inc)
