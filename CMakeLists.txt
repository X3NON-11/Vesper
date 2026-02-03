cmake_minimum_required(VERSION 4.0)
project(vesper)

# Create a library from your source files
add_library(vesper
    src/http/HttpServer.cpp
    src/http/HttpConnection.cpp
    src/tcp/TcpServer.cpp
    src/http/radixTree.cpp
    src/utils/urlEncoding.cpp
)

# Specify include directories for the library
target_include_directories(vesper PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# ONLY FOR TESTING PURPOSES
add_executable(vesper-app examples/main.cpp)
target_link_libraries(vesper-app vesper)
