# examples/CMakeLists.txt


add_executable(stream_parsing
    parse-from-stream/main.c
)

add_executable(file_parsing
    parse-from-file/main.c
)
target_link_libraries(stream_parsing PRIVATE tracex_core)
target_link_libraries(file_parsing PRIVATE tracex_core)
