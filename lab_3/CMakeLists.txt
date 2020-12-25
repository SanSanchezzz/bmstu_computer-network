cmake_minimum_required(VERSION 3.8)
project(lab_3)

# set(CMAKE_CXX_STANDARD 11) -std=c++0x
set(CMAKE_CXX_FLAGS "-std=c++14 -Wall -ggdb3 -pthread")
set(SOCKET_LIB
        socket/Socket.cpp)

set(INC
        server/engine.h
        server/poll_async.h
        server/server.h)

set(SRC
        server/engine.cpp
        server/poll_async.cpp
        server/server.cpp)

add_library(socket_lib STATIC ${SOCKET_LIB})

add_executable(cli client/client.cpp)
add_executable(app ${INC} ${SRC} server/main.cpp)

target_link_libraries(cli socket_lib)
target_link_libraries(app socket_lib)
