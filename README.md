# http-server
My little http library I have been working on in my free time

# Installation
1. Download & extract zip file from releases
2. Add to CMakeLists like this
```CMake
# Add the include directory for the library headers
target_include_directories(example1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/http-server)

# Link the static library
target_link_libraries(example1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/http-server/libhttp-server.a)
```

# Features
- start a server
- send plain text
- different endpoints

# How to use
```c++
#include "http-server.h"

// ====================
// All functions
// ====================
void myHandler(http::HttpConnection& c);
void testEndpoint(http::HttpConnection& c);

int main() {
    // Start the server
    http::HttpServer server("localhost", 8080);

    // Route handlers
    server.GET("/", myHandler);       // Website endpoint
    server.GET("/test", testEndpoint);  // JSON endpoint

    server.run();
}

void myHandler(http::HttpConnection& c) {
  c.string("Hello World");
}

void testEndpoint(http::HttpConnection& c) {
  const char* json = R"(
    {
      "status": "OK",
      "message": "This is a test JSON response"
    }
  )";

  c.json(json);
  // c.data("application/json", Status::OK, json); Also possible
}
```

# Why?
Because I wanted to learn more about how http servers work :)
