cmake_minimum_required(VERSION 4.0)

project(MyProject LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(example1 src/main.cpp)

# ==============================
#    How to add the library
# ==============================

# Add the include directory for the library headers
target_include_directories(example1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/http-server)

# Link the static library
target_link_libraries(example1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/http-server/libhttp-server.a)

# IMPORTANT: This library version might be outdated, please download it from the releases