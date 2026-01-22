#include "include/server.h"

namespace http {    // HTTP-SERVER
    HttpServer::HttpServer(std::string ipAddress, int port) : TcpServer(ipAddress, port) {}
    
    HttpServer::~HttpServer() {
        if (serverThread.joinable()) {
            serverThread.join();
        }
    }

    void HttpServer::run() {
        serverThread = std::thread(&TcpServer::runServer, this);
    }

    void HttpServer::onClient(int client) {
        log(LogType::Info, "Client accepted");

        // Receive data from client
        const int bufferSize = 1024;
        char buffer[bufferSize] = {0};
        int valread = recv(client, buffer, bufferSize - 1, 0);
        if (valread < 0) {
            log(LogType::Warn, "Couldn't receive client data");
        }
        
        // Parse http data
        char method[10], clientEndpoint[256], version[10];
        if (!sscanf(buffer, "%s %s %s", method, clientEndpoint, version) == 3) { // https://cplusplus.com/reference/cstdio/sscanf/
            log(LogType::Warn, "Failed to parse http request");
        }

        HttpConnection connection(client);
        bool handled = false;

        for (auto endpoint : allEndpoints) {
            if (endpoint.endpoint == clientEndpoint &&
                endpoint.method == std::string(method)) {
                endpoint.handler(connection);
                handled = true;
                break;
            }
        }

        if (!handled) handler ? handler(connection) : connection.sendErrorNoHandler();

        connection.sendBuffer();
        close(client);
    }
    
    void HttpServer::setHandler(std::function<void(HttpConnection&)> h) {
        handler = std::move(h);
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
    void HttpServer::GET(std::string endpoint, std::function<void(HttpConnection&)> h) { createEndpoint("GET", endpoint, h); }
    void HttpServer::POST(std::string endpoint, std::function<void(HttpConnection&)> h) { createEndpoint("POST", endpoint, h); }
    void HttpServer::PUT(std::string endpoint, std::function<void(HttpConnection&)> h) { createEndpoint("PUT", endpoint, h); }
    void HttpServer::DELETE(std::string endpoint, std::function<void(HttpConnection&)> h) { createEndpoint("DELETE", endpoint, h); }
    void HttpServer::PATCH(std::string endpoint, std::function<void(HttpConnection&)> h) { createEndpoint("PATCH", endpoint, h); }
    void HttpServer::OPTIONS(std::string endpoint, std::function<void(HttpConnection&)> h) { createEndpoint("OPTIONS", endpoint, h); }
    void HttpServer::HEAD(std::string endpoint, std::function<void(HttpConnection&)> h) { createEndpoint("HEAD", endpoint, h); }
}