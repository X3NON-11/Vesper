#pragma once

#include <cstring>          // memset
#include <unistd.h>         // close
#include <sys/socket.h>     // socket, bind, listen
#include <netinet/in.h>     // sockaddr_in
#include <arpa/inet.h>      // htons, inet_aton
#include <functional>       // std::function
#include <string>           // std::string::npos
#include <thread>           // multithreading for different clients
#include <fcntl.h>          // fcntl make recv non blocking
#include <vector>           // Used for storing the client buffer
#include <coroutine>        // Used for the async io
#include <sys/epoll.h>

#include "../utils/logging.h"        // My own logging library/header
#include "../http/HttpConnection.h"
#include "../utils/threadPool.h"
#include "../async/awaiters.h"
#include "../async/eventLoop.h"
#include "../async/task.h"
#include "../async/eventLoop_fwd.h"

namespace vesper {
    // The foundation of the program
    // This handles the basic socket
    class TcpServer {
        protected: // Allows acces for subclasses
            int listenSocket; // Socket that listens for new connections
            int port; // The port the listenSocket runs on
            threadPool threads;

        public:
            TcpServer() = default; // Only creates a object
            virtual ~TcpServer(); // Allows overide for subclasses (HttpServer)

            // Listens for new connections (clients)
            // void runServer();
            async::Task runServer();
            // Logic on connection (overwritten in HttpServer)
            virtual async::Task onClient(int client);

            // sets everything up for a basic Tcp Server
            int startServer(std::string ipAddress, int port);
            // closes socket (is run in the TcpServer destructor)
            void closeServer();
            
            // Functions that use Linux only functions
            bool setSocketNonBlocking(int client);
    };
}