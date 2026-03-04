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

// At some point I want to implement asyncI/O for Windows as well, but right now it is too painful to actually do