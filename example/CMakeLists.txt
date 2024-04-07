project(example)

add_executable(chatserver example.cc)

target_link_libraries(chatserver tcp_server)