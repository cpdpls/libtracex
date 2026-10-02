add_library(tracex_labels STATIC
    src/tracex_labels_errno.c
    src/tracex_labels_obj.c
    src/tracex_labels_event.c
    src/tracex_labels_utils.c
    
)

add_library(tracex::labels ALIAS tracex_labels)

target_include_directories(tracex_labels
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/external/cjson
        ${CMAKE_SOURCE_DIR}/external/uthash/include
)

target_link_libraries(tracex_labels
    PUBLIC
        tracex_core              # labels needs the public core API
    PRIVATE
        cjson
)