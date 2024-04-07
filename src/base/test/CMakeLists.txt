project(text)

add_executable(basetest test.cc)

target_link_libraries(basetest tcp_server)