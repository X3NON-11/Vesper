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
        closeSocket(listenSocket);
    }
}

// Sets up a basic Tcp Socket
socketT TcpServer::startServer(std::string ipAddress, int port) {
#ifdef _WIN32
    // https://stackoverflow.com/questions/37266805/tcpclient-class-in-c-microsoft-visual-studio-example
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        log(LogType::Error, "WSAStartup failed");
        return 1;
    }

    // Create a socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        log(LogType::Error, "Socket creation failed: " + WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Bind the socket to an IP address and port
    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr("127.0.0.1"); // Replace with desired IP
    service.sin_port = htons(55555);                  // Port number

    if (bind(serverSocket, (sockaddr *)&service, sizeof(service)) ==
        SOCKET_ERROR) {
        log(LogType::Error, "Bind failed " + WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 1) == SOCKET_ERROR) {
        log(LogType::Error, "Listen failed: " + WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
#else
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
#endif

    return 0;
}

async::Task TcpServer::runServer() {
    while (true) {
        socketT client = co_await async::AcceptAwaiter{listenSocket};

        if (client < 0)
            continue;

        setSocketNonBlocking(client);

        onClient(client); // starts another coroutine
    }
}

// Just a basic placeholder
// Is overwritten in HttpServer
async::Task TcpServer::onClient(socketT client) {
    log(LogType::Info, "Client accepted");
    const char *message = "No Handler parsed\n";
    send(client, message, strlen(message), 0);
    closeSocket(client);
    co_return;
}

// Functions that use Linux only functions
bool TcpServer::setSocketNonBlocking(socketT client) {
#ifdef _WIN32
    u_long iMode = 1;
    if (ioctlsocket(client, FIONBIO, &iMode) != NO_ERROR) {
        log(LogType::Warn, "Failed to set socket to non blocking");
    }
#else
    int flags = fcntl(client, F_GETFL, 0);
    if (flags == -1) {
        log(LogType::Warn, "fcntl F_GETFL");
        return false;
    }

    if (fcntl(client, F_SETFL, flags | O_NONBLOCK) == -1) {
        log(LogType::Warn, "fcntl F_SETFL");
        return false;
    }
#endif

    return true;
}
} // namespace vesper