cmake_minimum_required(VERSION 4.0)
project(http-server)

# Create a library from your source files
add_library(http-server
    src/HttpServer.cpp
    src/HttpConnection.cpp
    src/TcpServer.cpp
)

# Specify include directories for the library
target_include_directories(http-server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# ONLY FOR TESTING PURPOSES
# add_executable(http-server-app examples/main.cpp)
# target_link_libraries(http-server-app http-server)