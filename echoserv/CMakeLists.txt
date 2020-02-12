cmake_minimum_required(VERSION 3.0)

# set the project name
project(EchoServer)

# add the executable
add_executable(server echoserv.c helper.c)
add_executable(client echoclient.c helper.c)
