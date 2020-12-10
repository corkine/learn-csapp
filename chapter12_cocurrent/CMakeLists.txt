project(socket)
add_executable(socket librio.c libserver.c)
add_executable(echo_client echo_client.c)