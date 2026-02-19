.. Vesper documentation master file, created by
   sphinx-quickstart on Mon Feb  9 15:55:54 2026.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Vesper Documentation
====================

A Gin inspired http server library for C++

Overview
=======

| **Purpose of Vesper**
| Vesper is an HTTP server library for C++ inspired by Gin (a popular Go library). It was created to fill the gap of similar, easy-to-use libraries in C++ with clean and intuitive syntax.

| **Project Scope**
| Our goal is to recreate the functionality and simplicity of Gin while leveraging the power and features of C++.

| **Intended Audience**
| Vesper is designed for anyone interested in building HTTP servers in C++, especially those looking for a Gin-like experience in C++.

Features
========
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

How to use
==========
.. code-block:: cpp

    #include <vesper/vesper.h>
    
    // ====================
    // All functions
    // ====================
    void myHandler(http::HttpConnection& c);
    void testEndpoint(http::HttpConnection& c);
    
    int main() {
        // Start the server
        vesper::HttpServer server;
    
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
          "message": "This is a test.SON response"
        }
        )";
    
        c.json(json);
        // c.data("application/json", Status::OK, json); Also possible
    }
    
.. toctree::
   :maxdepth: 2
   :caption: Contents:

   Overview
   Documentation
   ProjectStructure