<div align="center">
<pre>
▗▖ ▗▖▗▄▄▄▖▗▄▄▄▖▗▄▄▖       ▗▄▄▖▗▄▄▄▖▗▄▄▖ ▗▖  ▗▖▗▄▄▄▖▗▄▄▖ 
▐▌ ▐▌  █    █  ▐▌ ▐▌     ▐▌   ▐▌   ▐▌ ▐▌▐▌  ▐▌▐▌   ▐▌ ▐▌
▐▛▀▜▌  █    █  ▐▛▀▘  ▀▀▀  ▝▀▚▖▐▛▀▀▘▐▛▀▚▖▐▌  ▐▌▐▛▀▀▘▐▛▀▚▖
▐▌ ▐▌  █    █  ▐▌        ▗▄▄▞▘▐▙▄▄▖▐▌ ▐▌ ▝▚▞▘ ▐▙▄▄▖▐▌ ▐▌
                                                   
</pre>
</div>
My little gin inspired http library I have been working on in my free time


# Installation
1. Download zip file from releases
2. Add to CMakeLists like this
```CMake
# Add the include directory for the library headers
target_include_directories(example1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/http-server)

# Link the static library
target_link_libraries(example1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/http-server/libhttp-server.a)
```

# Features
- start a server
- send plain text, json, etc.
- endpoints
- middleware
- receive client data through body
- querys
- URL parameters

# How to use
```c++
#include <http-server.h>

// ====================
// All functions
// ====================
void myHandler(http::HttpConnection& c);
void testEndpoint(http::HttpConnection& c);

int main() {
    // Start the server
    http::HttpServer server;

    // Route handlers
    server.GET("/", myHandler);       // Hello World endpoint
    server.GET("/test", testEndpoint);  // JSON endpoint

    server.run("localhost", 8080);
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

# Documentation
1. Create Endpoints
Use the functions GET, POST, PUT, DELETE, PATCH, OPTIONS, HEAD on the server object to create a new Endpoint.
```C++
server.GET("/", handler);
```
2. Creare Middleware
The function use() sets the given middleware for all endpoints.
```C++
server.use(handler);
```
Creating a middleware for one specific endpoint can be done by parsing it before the handler on endpoint creation.
```C++
server.GET("/", middlewareHandler, handler);
```
3. Send plain text, json etc
Can be done by using the function data() in which you can specify everything yourself.
```C++
c.data("text/plain", Status::OK, "Hello World");
```
But there are shortcuts using functions like string, json, status.
```C++
c.string("Hello World"); // When not giving a status code it defaults to 200
```
4. Querys
Querys can be used with the query() function, which takes the query name and returns the content.
For example in the URL it can be used as /endpoint?queryName=queryBody
```C++
std::string message = c.query("message");
```
5. URL Parameters
If querys aren't enough you can use URL parameters using the param() function.
It takes in the param name and gives back the content, but for it to work you first need to specify
the position of the parameter in the URL on endpoint creation using :
```C++
server.GET("/:test", handler);
// LATER IN THE HANDLER
std::string message = c.param("test");
```
