add_library(labels_engine STATIC
    src/labels_engine.c
    src/labels_engine_utils.c
    
)

add_library(tracex::labels_engine ALIAS labels_engine)

target_include_directories(labels_engine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/external/cjson
        ${CMAKE_SOURCE_DIR}/external/uthash/include
)

target_link_libraries(labels_engine
    PUBLIC
        tracex_core
    PRIVATE
        cjson
)