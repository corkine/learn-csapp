cmake_minimum_required(VERSION 3.10)
project(learnC C)

set(CMAKE_C_STANDARD 99)

add_executable(learnC chapter10_io/read.c chapter11_socket/hex2dd.c chapter09_memory/malloc_demo.c chapter09_memory/mmapcopy.c chapter08_exception/snooze.c chapter11_socket/socket_demo.c chapter11_socket/librio.c chapter11_socket/libserver.c chapter11_socket/echo_client.c chapter11_socket/librio.h chapter11_socket/echo_server.c chapter11_socket/tiny.c chapter11_socket/adder.c)
