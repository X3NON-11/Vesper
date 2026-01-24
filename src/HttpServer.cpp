#include "include/server.h"

namespace http {    // HTTP-SERVER
    HttpServer::HttpServer() : TcpServer() {}
    
    HttpServer::~HttpServer() {
        if (serverThread.joinable()) {
            serverThread.join();
        }
    }

    void HttpServer::run(std::string ipAddress, int port) {
        setupLogger();
        log(LogType::Info, "Starting Server");
        if (ipAddress == "localhost") ipAddress = "127.0.0.1";
        startServer(ipAddress, port);
        serverThread = std::thread(&TcpServer::runServer, this);
    }

    void HttpServer::onClient(int client) {
        // Receive data from client
        const int bufferSize = 1024;
        char buffer[bufferSize] = {0};
        int valread = recv(client, buffer, bufferSize - 1, 0);
        if (valread < 0) {
            log(LogType::Warn, "Couldn't receive client data");
        }
        
        // Parse http data
        char method[10], clientEndpoint[256], version[10];
        if (sscanf(buffer, "%s %s %s", method, clientEndpoint, version) != 3) { // https://cplusplus.com/reference/cstdio/sscanf/
            log(LogType::Warn, "Failed to parse http request");
        }

        if (strcmp(clientEndpoint, "/favicon.ico") == 0) {
            log(LogType::Info, "Ignoring favicon request");
            close(client);
            return;
        }

        HttpConnection connection(client);
        bool handled = false;

        // MiddleWare
        runMiddlewares(connection, clientEndpoint, method, 0, [&]() {
            for (auto& endpoint : allEndpoints) {
                if (endpoint.endpoint == clientEndpoint && endpoint.method == method) {
                    endpoint.handler(connection);
                    handled = true;
                    break;
                }
            }
        });

        if (!handled) connection.sendErrorNoHandler();

        connection.sendBuffer();
        close(client);

        // On the bottom so favicon.ico is skipped
        log(LogType::Info, "Client accepted");
    }
    
    void HttpServer::createEndpoint(std::string method, std::string endpoint, std::function<void(HttpConnection&)> h) {
        Endpoints newEndpoint;
        newEndpoint.method = method;
        newEndpoint.endpoint = endpoint;
        newEndpoint.handler = h;
        allEndpoints.push_back(newEndpoint);
        log(LogType::Info, "Succesfully created endpoint [" + endpoint + "]");
    }

    // Every Method for http
    void HttpServer::GET(std::string endpoint, Functions... middlewareHandlers, std::function<void(HttpConnection&)> h) { createEndpoint("GET", endpoint, h); }
    void HttpServer::POST(std::string endpoint, std::function<void(HttpConnection&)> h) { createEndpoint("POST", endpoint, h); }
    void HttpServer::PUT(std::string endpoint, std::function<void(HttpConnection&)> h) { createEndpoint("PUT", endpoint, h); }
    void HttpServer::DELETE(std::string endpoint, std::function<void(HttpConnection&)> h) { createEndpoint("DELETE", endpoint, h); }
    void HttpServer::PATCH(std::string endpoint, std::function<void(HttpConnection&)> h) { createEndpoint("PATCH", endpoint, h); }
    void HttpServer::OPTIONS(std::string endpoint, std::function<void(HttpConnection&)> h) { createEndpoint("OPTIONS", endpoint, h); }
    void HttpServer::HEAD(std::string endpoint, std::function<void(HttpConnection&)> h) { createEndpoint("HEAD", endpoint, h); }

    void HttpServer::use(std::function<void(HttpConnection&)> handler) {
        Middleware newMiddleware;
        newMiddleware.endpoint = "ALL";
        newMiddleware.method =  "ALL";
        newMiddleware.handler = handler;
        newMiddleware.nextHandler = nullptr;
        allMiddleware.push_back(newMiddleware);
    }

    void HttpServer::setMiddleware(std::string endpoint, std::string method, std::function<void(HttpConnection&)> handler) {
        Middleware newMiddleware;
        newMiddleware.endpoint = endpoint;
        newMiddleware.method =  method;
        newMiddleware.handler = handler;
        newMiddleware.nextHandler = nullptr;
        allMiddleware.push_back(newMiddleware);
    }
    
    void HttpServer::runMiddlewares(HttpConnection& connection, std::string clientEndpoint, std::string method, int index, std::function<void()> finalHandler) {
        if (index >= allMiddleware.size()) {
            finalHandler();
            return;
        }

        auto mw = allMiddleware[index];

        bool matches =
            (mw.endpoint == clientEndpoint && mw.method == method) ||
            (mw.endpoint == clientEndpoint && mw.method == "ALL") ||
            (mw.endpoint == "ALL" && mw.method == "ALL") ||
            (mw.endpoint == "ALL" && mw.method == method);

        if (matches) {
            // Execute and go to next middleware
            connection.setNext([&, index]() {
                runMiddlewares(connection, clientEndpoint, method, index + 1, finalHandler);
            });
            mw.handler(connection);
        } else {
            // Skip this middleware
            runMiddlewares(connection, clientEndpoint, method, index + 1, finalHandler);
        }
    }
}