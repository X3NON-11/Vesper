#include "../include/tcp/TcpServer.h"

namespace vesper {
// ==========
// TCP-SERVER
// ==========

// Automatically clean up when closing
TcpServer::~TcpServer() {
    closeServer();
    threads.stop();
}

// Close Tcp Socket
void TcpServer::closeServer() {
    if (listenSocket >= 0) {
        close(listenSocket);
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

    // Initialize Socket
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        log(LogType::Error, "Couldn't initialize socket");
    }

    // Enable socket reuse
    int opt = 1; // Enables this option
    if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) <
        0) {
        log(LogType::Error, "Couldn't enable option for socket reuse");
        return -1;
    }

    // Bind socket to ip-address
    int bindStatus = bind(listenSocket, (struct sockaddr *)&server_addr,
                          sizeof(server_addr));
    if (bindStatus < 0) {
        log(LogType::Error, "Couldn't bind socket to ip-address");
    }

    // Listen on socket
    if (listen(listenSocket, 5) != 0) {
        log(LogType::Error, "Couldn't listen on socket");
    }

    return 0;
}

async::Task TcpServer::runServer() {
    while (true) {
        int client = co_await async::AcceptAwaiter{listenSocket};

        if (client < 0)
            continue;

        setSocketNonBlocking(client);

        onClient(client); // starts another coroutine
    }
}

// Just a basic placeholder
// Is overwritten in HttpServer
async::Task TcpServer::onClient(int client) {
    log(LogType::Info, "Client accepted");
    const char *message = "No Handler parsed\n";
    send(client, message, strlen(message), 0);
    close(client);
    co_return;
}

// Functions that use Linux only functions
bool TcpServer::setSocketNonBlocking(int client) {
    int flags = fcntl(client, F_GETFL, 0);
    if (flags == -1) {
        log(LogType::Warn, "fcntl F_GETFL");
        return false;
    }

    if (fcntl(client, F_SETFL, flags | O_NONBLOCK) == -1) {
        log(LogType::Warn, "fcntl F_SETFL");
        return false;
    }

    return true;
}
} // namespace vesper