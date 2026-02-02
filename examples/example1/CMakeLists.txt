cmake_minimum_required(VERSION 4.0)

project(MyProject LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(example1 src/main.cpp)

# ==============================
#    How to add the library
# ==============================

include(FetchContent)

FetchContent_Declare(
  vesper
  GIT_REPOSITORY https://github.com/DieOlive/Vesper
  GIT_TAG prerelease_0.94
)

target_link_libraries(example1 PRIVATE vesper)