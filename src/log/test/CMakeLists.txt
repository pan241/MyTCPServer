project(log_test)

add_executable(log_test test.cc)

target_link_libraries(log_test tcp_server)