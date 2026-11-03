enable_language(CXX)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Charts)

add_executable(stream_parsing
    parse-from-stream/main.c
)

add_executable(file_parsing
    parse-from-file/main.c
)

add_executable(qt_stats_viewer
    
)

target_link_libraries(qt_stats_viewer PRIVATE
    Qt6::Widgets
    tracex_core
    labels_engine
    tracex_stats
)

target_link_libraries(stream_parsing PRIVATE
    tracex_core
    labels_engine
    tracex_stats
)

target_link_libraries(file_parsing PRIVATE
    tracex_core
    labels_engine
    tracex_stats
)

target_link_libraries(qt_stats_viewer PRIVATE
    Qt6::Widgets
    tracex_core
    labels_engine
    tracex_stats
)