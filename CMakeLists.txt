cmake_minimum_required(VERSION 4.0)
project(http-server)

add_executable(http-server 
    src/main.cpp
    src/HttpConnection.cpp
    src/HttpServer.cpp
    src/TcpServer.cpp
)