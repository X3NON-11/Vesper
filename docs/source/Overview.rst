Overview
=======

| **Purpose of Vesper**
| Vesper is an HTTP server library for C++ inspired by Gin (a popular Go library). It was created to fill the gap of similar, easy-to-use libraries in C++ with clean and intuitive syntax.

| **Project Scope**
| Our goal is to recreate the functionality and simplicity of Gin while leveraging the power and features of C++.

| **Intended Audience**
| Vesper is designed for anyone interested in building HTTP servers in C++, especially those looking for a Gin-like experience in C++.

Requirements/Recommendations for using Vesper
=============================================
- C++ >= 20
- CMake
- Ninja (any other CMake Generator is also fine)

Installation
============

| **For basic use**
| Add this to your CMakeLists

.. code-block:: cmake

    include(FetchContent)
    
    FetchContent_Declare(
      vesper
      GIT_REPOSITORY https://github.com/X3NON-11/Vesper
      GIT_TAG prerelease_0.95
    )
    
    FetchContent_MakeAvailable(vesper)
    
    target_link_libraries(example1 PRIVATE vesper)
    
| Or if you want a manually downloaded Version
| 1. Download zip file from releases
| 2. Add to CMakeLists like this

.. code-block:: cmake

    # Add the include directory for the library headers
    target_include_directories(example1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/vesper)

    # Link the static library
    target_link_libraries(example1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/vesper/libvesper.a)
    
| **For Development**
| 1. Download the library (git clone git@github.com:X3NON-11/Vesper.git)
| 2. Go into the folder and then you are already done
| => Recommended: The CMakeLists is set up so you build a library and executable (for testing), so for ease of testing you can use the main.cpp under Vesper/examples