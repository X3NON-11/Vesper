cmake_minimum_required(VERSION 4.0)
project(example1 LANGUAGES CXX)

# Normally you don't need to specify the library, but now you do because it isn't under the CMakeLists directory
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. Vesper)

add_executable(example1 src/main.cpp)
target_link_libraries(example1 PRIVATE Vesper::Vesper)