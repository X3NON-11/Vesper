# http-server
My little http library I have been working on in my free time

# Installation
1. The project is not finished and currently not a library

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
void myHandler(http::HttpConnection& conn);
void testEndpoint(http::HttpConnection& conn);

int main() {
    // Start the server
    http::HttpServer server("localhost", 8080);

    // Route handlers
    server.setHandler(myHandler);       // Default handler for / & fallback
    server.GET("/test", testEndpoint);  // JSON endpoint

    server.run();
}

void myHandler(http::HttpConnection& conn) {
  conn.sendPlainText("Hello World");
}

void testEndpoint(http::HttpConnection& conn) {
  const char* json = R"(
    {
      "status": "OK",
      "message": "This is a test JSON response"
    }
  )";

  conn.data("application/json", Status::OK, json);
}
```

# Why?
Because I wanted to learn more about how http servers work :)
