add_library(tracex_resolver STATIC
    src/tracex_labels_obj.c
    src/tracex_labels_event.c
    src/tracex_labels_utils.c
    
)

add_library(tracex::resolver ALIAS tracex_resolver)

target_include_directories(tracex_resolver
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/external/cjson
        ${CMAKE_SOURCE_DIR}/external/uthash/include
)

target_link_libraries(tracex_resolver
    PUBLIC
        tracex_core
    PRIVATE
        cjson
)