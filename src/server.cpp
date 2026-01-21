#include "include/server.h"

/* Inspiration
https://github.com/bozkurthan/Simple-TCP-Server-Client-CPP-Example/blob/master/tcp-Server.cpp
https://www.geeksforgeeks.org/c/tcp-server-client-implementation-in-c/
https://man7.org/linux/man-pages/man2/bind.2.html etc.
*/

namespace http {    // TCP-SERVER
    TcpServer::TcpServer(std::string ipAddress, int port) {
        log(LogType::Info, "Starting Server");
        if (ipAddress == "localhost") ipAddress = "127.0.0.1";
        startServer(ipAddress, port);
    }

    TcpServer::~TcpServer() {
        closeServer();
        log(LogType::Info, "Closed Server");
    }

    void TcpServer::closeServer() {
        if (listenSocket >= 0) {
            close(listenSocket);
            log(LogType::Info, "Closing Socket");
        }
    }

    int TcpServer::startServer(std::string ipAddress, int port) {
        struct sockaddr_in server_addr;
        std::memset(&server_addr, 0, sizeof(server_addr)); // Zero-initialize
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        inet_aton(ipAddress.c_str(), &server_addr.sin_addr);
        
        log(LogType::Info, "Initialize socket");
        listenSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (listenSocket < 0) {
            log(LogType::Error, "Couldn't initialize socket");
        }

        log(LogType::Info, "Enable socket reuse");
        int opt = 1; // Enables this option
        if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            log(LogType::Error, "Couldn't enable option for socket reuse");
            return -1;
        }

        log(LogType::Info, "Bind socket to ip-address");
        int bindStatus = bind(listenSocket, (struct sockaddr*) &server_addr, 
        sizeof(server_addr));
        if(bindStatus < 0) {
            log(LogType::Error, "Couldn't bind socket to ip-address");
        }

        log(LogType::Info, "Listen on socket");
        if (listen(listenSocket, 5) != 0) {
            log(LogType::Error, "Couldn't listen on socket");
        }

        return 0;
    }

    void TcpServer::runServer() {
        log(LogType::Info, "Accept client");
        while (true) {
            int client = accept(listenSocket, nullptr, nullptr);
            if (client < 0) {
                log(LogType::Error, "Couldn't accept client");
            }

            std::thread callback(&TcpServer::onClient, this, client);
            callback.detach();
        }
    }

    void TcpServer::onClient(int client) {
        log(LogType::Info, "Client accepted");
        HttpConnection connection(client);
        connection.process();
        close(client);
    }
}

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
        char method[10], endpoint[256], version[10];
        if (sscanf(buffer, "%s %s %s", method, endpoint, version) == 3) { // https://cplusplus.com/reference/cstdio/sscanf/
            log(LogType::Info, "Method: " + std::string(method) + " Endpoint: " + endpoint + " Version: " + version);
        } else {
            log(LogType::Warn, "Failed to parse http request");
        }

        HttpConnection connection(client);
        if (handler) {
            handler(connection);
        } else { connection.process(); }

        close(client);
    }
    
    void HttpServer::setHandler(std::function<void(HttpConnection&)> h) {
        handler = std::move(h);
    }
}

namespace http {    // HTTP-CONNECTION responsible for translating abstractions to tcp usable format
    HttpConnection::HttpConnection(int client) : client(client) {}

    void HttpConnection::process() {
        sendPlainText(HttpResponse::StatusCodes::INTERNAL_SERVER_ERROR, "No handler parsed");
        log(LogType::Warn, "No handler parsed");
    }

    void HttpConnection::sendPlainText(HttpResponse::StatusCodes status, std::string body) {
        HttpResponse response(status, body);

        std::string http = response.toHttpString();
        send(client, http.c_str(), http.size(), 0);
    }

    void HttpConnection::sendPlainText(std::string body) {
        sendPlainText(HttpResponse::StatusCodes::OK, body);
    }
}

namespace http {    // HTTP-RESPONSE used to convert http logic to tcp logic
    HttpResponse::HttpResponse(StatusCodes status, std::string body) : status(status), body(body) {}

    std::string HttpResponse::toHttpString() {
        return "HTTP/1.1 " +
            std::to_string(static_cast<int>(status)) + " " +
            statusToString(status) + "\r\n" +
            "Content-Length: " + std::to_string(body.size()) + "\r\n" +
            "\r\n" +
            body;
    }
    
    std::string HttpResponse::statusToString(StatusCodes status) {
        switch (status) {
            case StatusCodes::OK: return "OK";
            case StatusCodes::BAD_REQUEST: return "Bad Request";
            case StatusCodes::UNAUTHORIZED: return "Unauthorized";
            case StatusCodes::FORBIDDEN: return "Forbidden";
            case StatusCodes::NOT_FOUND: return "Not Found";
            case StatusCodes::TOO_MANY_REQUESTS: return "Too Many Requests";
            case StatusCodes::INTERNAL_SERVER_ERROR: return "Internal Server Error";
            default: return "Unknown";
        }
    }
}