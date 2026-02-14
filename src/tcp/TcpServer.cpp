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

// Should run multithreaded (HttpServer does that)
void TcpServer::runServer() {
    // Infinitly accepts new clients on socket and forwards them by executing
    // onClient()
    while (true) {
        int client = accept(listenSocket, nullptr, nullptr);
        if (client < 0) {
            log(LogType::Error, "Couldn't accept client");
        }

        threads.newTask([this, client]() { onClient(client); });
    }
}

// Just a basic placeholder
// Is overwritten in HttpServer
void TcpServer::onClient(int client) {
    log(LogType::Info, "Client accepted");
    const char *message = "No Handler parsed\n";
    send(client, message, strlen(message), 0);
    close(client);
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

bool TcpServer::receiveRequest(int client, std::string &request,
                               std::vector<char> &buffer) {
    while (request.find("\r\n\r\n") == std::string::npos) {
        int r = recv(client, buffer.data(), buffer.size(), 0);
        if (r > 0) {
            request.append(buffer.data(), r);
        } else if (r == 0) {
            log(LogType::Warn, "Client closed connection");
            return false;
        } else { // r < 0
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // no data yet -> wait a little, then retry
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            } else {
                log(LogType::Warn, "recv error");
                return false;
            }
        }

        // Prevent header abuse
        if (request.size() > 16 * 1024) {
            log(LogType::Warn, "HTTP headers too large");
            return false;
        }
    }
    return true;
}
bool TcpServer::receivePostData(int client, std::vector<char> &buffer,
                                std::string postData, int timeout,
                                int contentLength) {
    auto start = std::chrono::steady_clock::now();
    while (postData.size() < contentLength) {
        int r = recv(client, buffer.data(), buffer.size(), 0);
        if (r > 0) {
            postData.append(buffer.data(), r);
        } else if (r == 0) {
            log(LogType::Warn, "Client closed during body");
            return false;
        } else {
            if (errno == EAGAIN ||
                errno == EWOULDBLOCK) { // Not a timeout — just no data yet
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            } else {
                log(LogType::Warn, "recv error");
                return false;
            }
        }

        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - start)
                .count() > timeout) {
            log(LogType::Warn, "Client took too long to send body");
            return false;
        }
    }
    return true;
}

} // namespace vesper