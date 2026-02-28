#pragma once

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    using socketT = SOCKET;
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    using socketT = int;
#endif

inline void closeSocket(socketT socket) {
    #ifdef _WIN32
        closesocket(socket);
    #else
        close(socket);
    #endif
}