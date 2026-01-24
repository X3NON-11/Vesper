#include "include/server.h"

namespace http {    // HTTP-SERVER
    HttpServer::HttpServer() : TcpServer() {}
    
    // Close server automatically
    HttpServer::~HttpServer() {
        if (serverThread.joinable()) {
            serverThread.join();
        }
    }

    // Sets up & runs the server using the previously created objects
    void HttpServer::run(std::string ipAddress, int port) {
        setupLogger();
        log(LogType::Info, "Starting Server");
        if (ipAddress == "localhost") ipAddress = "127.0.0.1";
        startServer(ipAddress, port);
        serverThread = std::thread(&TcpServer::runServer, this);
    }

    // Decides when & at what endpoint to run the handlers
    void HttpServer::onClient(int client) {
        // Getting what endpoint, method client has/wants to later run the correct handler
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

        // Skip Website Logo if client connected with a browser
        if (strcmp(clientEndpoint, "/favicon.ico") == 0) {
            log(LogType::Info, "Ignoring favicon request");
            close(client);
            return;
        }

        HttpConnection connection(client);
        bool handled = false;

        // MiddleWare / All Handlers
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

        // On the bottom so favicon.ico is skipped in the logs
        log(LogType::Info, "Client accepted");
    }
    
    // Create & Save Endpoint to allEndpoints so it is handled in onClient()
    void HttpServer::createEndpoint(std::string method, std::string endpoint, std::function<void(HttpConnection&)> h) {
        Endpoints newEndpoint;
        newEndpoint.method = method;
        newEndpoint.endpoint = endpoint;
        newEndpoint.handler = h;
        allEndpoints.push_back(newEndpoint);
        log(LogType::Info, "Succesfully created endpoint [" + endpoint + "]");
    }

    // Middleware
    // Create a middleware by saving it to allMiddleware
    // This then gets processed in runMiddlewares()
    void HttpServer::setMiddleware(std::string endpoint, std::string method, std::function<void(HttpConnection&)> handler) {
        Middleware newMiddleware;
        newMiddleware.endpoint = endpoint;
        newMiddleware.method =  method;
        newMiddleware.handler = handler;
        newMiddleware.nextHandler = nullptr;
        allMiddleware.push_back(newMiddleware);
    }
    // Sets a global middleware
    void HttpServer::use(std::function<void(HttpConnection&)> handler) {
        setMiddleware("ALL", "ALL", handler);
    }
    
    // Recursive function that goes through every Middleware to decide what to run
    void HttpServer::runMiddlewares(HttpConnection& connection, std::string clientEndpoint, std::string method, int index, std::function<void()> finalHandler) {
        // If went through every Middleware run the given function
        // The given function is in onClient()
        // Which goes through every endpoint and runs the correct handler
        if (index >= allMiddleware.size()) {
            finalHandler();
            return;
        }

        auto mw = allMiddleware[index];

        // Requirements for a middleware to run
        bool matches =
            (mw.endpoint == clientEndpoint && mw.method == method) ||
            (mw.endpoint == clientEndpoint && mw.method == "ALL") ||
            (mw.endpoint == "ALL" && mw.method == "ALL") ||
            (mw.endpoint == "ALL" && mw.method == method);

        if (matches) {
            // Go to the next middleware (recursive)
            connection.setNext([&, index]() {
                runMiddlewares(connection, clientEndpoint, method, index + 1, finalHandler);
            });
            // Execute current handler
            mw.handler(connection);
        } else {
            // Skip this middleware
            runMiddlewares(connection, clientEndpoint, method, index + 1, finalHandler);
        }
    }
}