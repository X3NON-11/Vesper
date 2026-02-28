cmake_minimum_required(VERSION 4.0)
project(vesper)
include(FetchContent)

# Create a library from your source files
add_library(vesper
    src/http/HttpServer.cpp
    src/http/HttpConnection.cpp
    src/tcp/TcpServer.cpp
    src/http/radixTree.cpp
    src/utils/urlEncoding.cpp
    src/utils/threadPool.cpp
)

# Specify include directories for the library
target_include_directories(vesper PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Add Flag like this because it else conflicts with zlib
target_compile_options(vesper PRIVATE -fcoroutines)

# ONLY FOR TESTING PURPOSES
add_executable(vesper-app examples/main.cpp)
# Add Flag like this because it else conflicts with zlib
target_compile_options(vesper-app PRIVATE -fcoroutines)
target_link_libraries(vesper-app vesper)

# ==================
# External Libraries
# ==================

FetchContent_Declare(
  zlib
  GIT_REPOSITORY https://github.com/madler/zlib.git
  GIT_TAG v1.3.1
)
FetchContent_MakeAvailable(zlib)
target_link_libraries(vesper PRIVATE zlib)

FetchContent_Declare(
  glaze
  GIT_REPOSITORY https://github.com/stephenberry/glaze.git
  GIT_TAG v7.0.2
)
set(GLAZE_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(GLAZE_ENABLE_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(GLAZE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glaze)
target_link_libraries(vesper PUBLIC glaze::glaze)