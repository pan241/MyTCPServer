cmake_minimum_required(VERSION 3.0)
project(TcpServer)

include_directories(
    ${PROJECT_SOURCE_DIR}/src/base
    ${PROJECT_SOURCE_DIR}/src/http
    ${PROJECT_SOURCE_DIR}/src/log
    ${PROJECT_SOURCE_DIR}/src/net
    ${PROJECT_SOURCE_DIR}/src/net/poller
    ${PROJECT_SOURCE_DIR}/src/timer
    ${PROJECT_SOURCE_DIR}/include
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

aux_source_directory(${PROJECT_SOURCE_DIR}/src/base SRC_BASE)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/log SRC_LOG)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/timer SRC_TIMER)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/net SRC_NET)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/net/poller SRC_POLLER)

#add_subdirectory(src/base/test)
#add_subdirectory(src/http)
add_subdirectory(src/log/test)
add_subdirectory(example)
#add_subdirectory(src/timer)

set(CMAKE_BUILD_TYPE "Debug")

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

set(TEST_PATH ${PROJECT_SOURCE_DIR}/bin)

add_library(tcp_server SHARED ${SRC_BASE} ${SRC_LOG}  ${SRC_NET} ${SRC_TIMER} ${SRC_POLLER})