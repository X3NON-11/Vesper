<div align="center">
  
  <img width="398" height="153" alt="VesperLogo" src="https://github.com/user-attachments/assets/4c61c113-be77-4a67-90a1-b829be1ecc0e" />

 *A Gin inspired http server library for C++*
</div>

  
# Features
- start a server
- send plain text, json, etc.
- endpoints
- middleware
- receive client data through the body
- querys
- URL parameters
- get client headers (e.g. Auth Headers)
- router groups
- redirects

# How to use
```c++
#include <vesper/vesper.h>

// ====================
// All functions
// ====================
void myHandler(vesper::HttpConnection& c);
void testEndpoint(vesper::HttpConnection& c);

int main() {
    // Start the server
    vesper::HttpServer server;

    // Route handlers
    server.GET("/", myHandler);       // Hello World endpoint
    server.GET("/test", testEndpoint);  // JSON endpoint

    server.run("localhost", 8080);
}

void myHandler(vesper::HttpConnection& c) {
    c.string("Hello World");
}

void testEndpoint(vesper::HttpConnection& c) {
    const char* json = R"(
    {
      "status": "OK",
      "message": "This is a test.SON response"
    }
    )";

    c.json(json);
    // c.data("application/json", Status::OK, json); Also possible
}
```

# Installation - Linux
Add this to your CMakeLists

```CMake
    include(FetchContent)
    
    FetchContent_Declare(
      vesper
      GIT_REPOSITORY https://github.com/X3NON-11/Vesper
      GIT_TAG prerelease_0.95
    )
    
    FetchContent_MakeAvailable(vesper)
    
    target_link_libraries(test PRIVATE vesper::vesper)
```
    
Or if you want a manually downloaded Version
1. Download zip file from releases
2. Add to CMakeLists like this

```CMake  
    add_subdirectory(libs/vesper)    
    target_link_libraries(test PRIVATE vesper::vesper)
```

# Installation - Windows
We currently don't have native Windows support, but you can use the docker folder (Dockerfile)
as a starting point for your project, so it can run cross plattform

# Documentation
**Important**
For further documentation download the repository and open the sphinx documentation 
under the build folder (e.g. on Linux then run ./docs.sh)
**Create Endpoints**  
Use the functions GET, POST, PUT, DELETE, PATCH, OPTIONS, HEAD on the server object to create a new Endpoint.
```C++
server.GET("/", handler);
```
**Create Middleware**  
The function use() sets the given middleware for all endpoints.
```C++
server.use(handler);
```
Creating a middleware for one specific endpoint can be done by parsing it before the handler on endpoint creation.
```C++
server.GET("/", middlewareHandler, handler);
```
**Send plain text, json etc**  
Can be done by using the function data() in which you can specify everything yourself.
```C++
c.data("text/plain", Status::OK, "Hello World");
```
But there are shortcuts using functions like string, json, status.
```C++
c.string("Hello World"); // When not giving a status code it defaults to 200
```
**Querys**  
Querys can be used with the query() function, which takes the query name and returns the content.
For example in the URL it can be used as /endpoint?queryName=queryBody
```C++
std::string message = c.query("message");
```
**URL Parameters**  
If querys aren't enough you can use URL parameters using the param() function.
It takes in the param name and gives back the content, but for it to work you first need to specify
the position of the parameter in the URL on endpoint creation using :
```C++
server.GET("/:test", handler);
// LATER IN THE HANDLER
std::string message = c.param("test");
```
**Get Headers**  
Useful for Auth Headers etc. Can be use with the getHeader() function which takes in the header name and gives back the content
```C++
std::string message = c.getHeader("message");
```

**Router Grous**  
Useful when you want to set a middleware for all endpoints beneath another. When for example given the endpoint /test it lets you set middleware for all endpoints beneath using g.use(middleware) or create new endpoints with g.GET("/example", handler) leading to the creation of the endpoint /test/example
```C++
vesper::Router g = server.group("/test");

```

**Redirects**  
Can be for example used when autherization failed and you want to send the user to the home page. Then you can use c.redirect("/") to redirect him
```C++
c.redirect("/");
```
