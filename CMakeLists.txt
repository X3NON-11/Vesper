cmake_minimum_required(VERSION 3.21)
project(vesper LANGUAGES CXX)

include(FetchContent)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# =========================
# Dependencies
# =========================

option(VESPER_FETCH_DEPS "Fetch dependencies locally" ON)

if(VESPER_FETCH_DEPS)
    include(FetchContent)

    FetchContent_Declare(
        glaze
        GIT_REPOSITORY https://github.com/stephenberry/glaze.git
        GIT_TAG v7.0.2
    )

    FetchContent_MakeAvailable(glaze)

    find_package(ZLIB REQUIRED)
else()
    find_package(ZLIB REQUIRED)
    find_package(glaze CONFIG REQUIRED)
endif()

set(GLAZE_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(GLAZE_ENABLE_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(GLAZE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# =========================
# Library
# =========================

add_library(vesper
    src/http/HttpServer.cpp
    src/http/HttpConnection.cpp
    src/tcp/TcpServer.cpp
    src/http/radixTree.cpp
    src/utils/urlEncoding.cpp
    src/utils/threadPool.cpp
)

add_library(vesper::vesper ALIAS vesper)

target_include_directories(vesper
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

target_compile_features(vesper PUBLIC cxx_std_20)

target_link_libraries(vesper
    PUBLIC glaze::glaze
    PUBLIC ZLIB::ZLIB
)

install(TARGETS vesper
    EXPORT vesperTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT vesperTargets
    FILE vesperTargets.cmake
    NAMESPACE vesper::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vesper
)

# =========================
# Example Binary (for testing)
# =========================

add_executable(vesper-app examples/main.cpp)

target_link_libraries(vesper-app
    PRIVATE vesper::vesper
)