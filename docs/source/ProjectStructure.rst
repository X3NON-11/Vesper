Project Structure
=================

This document provides an overview of the structure of Vesper and explains how its components interact.

Overview
--------

After including the header:

.. code-block:: cpp

    #include <vesper/vesper.h>

you can create an HTTP server using the `vesper::HttpServer` class, which is a subclass of `TcpServer`.

The server uses a tree-based routing mechanism, and each client connection is handled via dedicated connection objects that encapsulate request and response data.

Class Hierarchy
---------------

.. graphviz::

    digraph G {
        TcpServer -> HttpServer;
        HttpServer -> HttpConnection;
        HttpConnection -> HttpRequest;
        HttpConnection -> HttpResponse;
    }

Main Components
---------------

TcpServer
~~~~~~~~~

- Base class for network servers.
- Manages listening on a TCP socket and accepting client connections.
- Provides the virtual function `onClient()` that handles new connections.
- Subclasses can override `onClient()` to implement protocol-specific behavior.

HttpServer
~~~~~~~~~~

- Subclass of `TcpServer`.
- Implements HTTP-specific logic.
- Exposes a simple interface for defining routes:

  .. code-block:: cpp

      server.GET("/test", handler);

- Internally uses `RadixTree` (defined in `RadixTree.cpp`) to store URL paths. `addURL()` is called to register new routes.
- The `run()` method starts the server and begins listening for connections.

HttpConnection
~~~~~~~~~~~~~~

- Represents a single HTTP client connection.
- Stores the context of the connection, including request and response objects.
- Provides an interface for handlers to send responses:

  .. code-block:: cpp

      void handler(HttpConnection& c) {
          c.string = "Hello, world!";
      }

HttpRequest
~~~~~~~~~~~

- Encapsulates the HTTP request data (method, headers, body, etc.).
- Accessible via `HttpConnection` during request handling.

HttpResponse
~~~~~~~~~~~~

- Encapsulates the HTTP response data (status, headers, body, etc.).
- Managed by `HttpConnection` and automatically sent when the handler completes.

Flow of a Client Request
------------------------

1. The server listens on a TCP socket (`TcpServer::run()`).
2. When a client connects:
   - The server accepts the connection and redirects it to a new socket.
   - `onClient()` is called (overridden in `HttpServer`).
3. `HttpServer::onClient()` handles HTTP parsing and request processing.
4. A `HttpConnection` object is created for the client.
5. The library executes the handler registered for the requested path.
6. The handler uses the `HttpConnection` context to send a response.
7. Once the handler finishes, the connection socket is closed.

Routing
-------

- Paths are stored in a radix tree (`RadixTree.cpp`) for efficient lookup.
- Routes are registered with `addURL()` internally when calling `GET`, `POST`, etc.
- Example:

  .. code-block:: cpp

      server.GET("/hello", [](HttpConnection& c) {
          c.string = "Hello!";
      });

Summary
-------

- `TcpServer` — network layer
- `HttpServer` — HTTP layer
- `HttpConnection` — request/response context
- `HttpRequest` / `HttpResponse` — encapsulated HTTP data
- `RadixTree` — efficient routing
