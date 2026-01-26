#include "include/server.h"

namespace http {    // TCP-SERVER
    // Automatically clean up when closing
    TcpServer::~TcpServer() {
        closeServer();
        log(LogType::Info, "Closed Server");
    }

    // Close Tcp Socket
    void TcpServer::closeServer() {
        if (listenSocket >= 0) {
            close(listenSocket);
            log(LogType::Info, "Closing Socket");
        }
    }

    // Sets up a basic Tcp Socket
    int TcpServer::startServer(std::string ipAddress, int port) {
        /*  ##### Inspiration #####

            https://github.com/bozkurthan/Simple-TCP-Server-Client-CPP-Example/blob/master/tcp-Server.cpp
            https://www.geeksforgeeks.org/c/tcp-server-client-implementation-in-c/
            https://man7.org/linux/man-pages/man2/bind.2.html etc.
        */

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

    // Should run multithreaded (HttpServer does that)
    void TcpServer::runServer() {
        log(LogType::Info, "Accept client");
        // Infinitly accepts new clients on socket and forwards them by executing onClient()
        while (true) {
            int client = accept(listenSocket, nullptr, nullptr);
            if (client < 0) {
                log(LogType::Error, "Couldn't accept client");
            }

            std::thread callback(&TcpServer::onClient, this, client);
            callback.detach();
        }
    }

    // Just a basic placeholder
    // Is overwritten in HttpServer
    void TcpServer::onClient(int client) {
        log(LogType::Info, "Client accepted");
        HttpConnection connection(client);
        connection.sendErrorNoHandler();
        close(client);
    }
}