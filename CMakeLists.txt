cmake_minimum_required(VERSION 4.0)
project(Vesper LANGUAGES CXX)

# =======
# LIBRARY
# =======

add_library(Vesper SHARED
    src/http/HttpConnection.cpp
    src/http/HttpServer.cpp
    src/http/radixTree.cpp
    src/tcp/TcpServer.cpp
    src/utils/threadPool.cpp
    src/utils/urlEncoding.cpp
)
add_library(Vesper::Vesper ALIAS Vesper)

target_compile_features(Vesper PUBLIC cxx_std_20)

target_include_directories(Vesper PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

option(USE_SYSTEM_LIBRARIES "Use system-installed glaze & zlib libraries" OFF)
if (USE_SYSTEM_LIBRARIES)
    find_package(glaze REQUIRED)
    find_package(ZLIB REQUIRED)
else()
    include(FetchContent)
    
    FetchContent_Declare(
      glaze
      GIT_REPOSITORY https://github.com/stephenberry/glaze.git
      GIT_TAG main
      GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(glaze)
    
    FetchContent_Declare(
        ZLIB
        GIT_REPOSITORY https://github.com/madler/zlib.git
        GIT_TAG develop
    )
    FetchContent_MakeAvailable(ZLIB)
    
endif()

target_link_libraries(Vesper PUBLIC
    glaze::glaze
    ZLIB::ZLIB
)

option(SKIP_TESTING "Skip the testing binaries" ON)
if (NOT SKIP_TESTING)
    # ==============
    # TESTING BINARY
    # ==============
    
    add_executable(Vesper-app examples/main.cpp)
    target_link_libraries(Vesper-app PRIVATE Vesper)
    
    # =====
    # GTEST
    # =====
    
    add_executable(Vesper-tests 
        tests/main.cpp
        tests/http/HttpConnection_test.cpp
        tests/http/HttpServer_test.cpp
        tests/http/radixTree_test.cpp
        tests/tcp/TcpServer_test.cpp
        tests/utils/urlEncoding_test.cpp
    )
    
    if (USE_SYSTEM_LIBRARIES)
        find_package(gtest REQUIRED)
    else()
        FetchContent_Declare(
            gtest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG main
        )
        FetchContent_MakeAvailable(gtest)
    endif()
    
    target_link_libraries(Vesper-tests PRIVATE
        gtest
    )
    
    target_link_libraries(Vesper-tests PRIVATE Vesper)
endif()