#pragma once

#include <vector>           // vector... why am I commentig this?
#include <cstdio>           // sscanf
#include <thread>           // multithreading for different clients
#include <iostream>         // idk
#include <cstring>          // memset
#include <unistd.h>         // close
#include <sys/socket.h>     // socket, bind, listen
#include <netinet/in.h>     // sockaddr_in
#include <arpa/inet.h>      // htons, inet_aton
#include <functional>       // std::function

#include "logging.h"        // My own logging library/header

namespace http {
    class TcpServer { // The foundation of the program
        protected: // Allows acces for subclasses
            int listenSocket;
            int port;

            int startServer(std::string ipAddress, int port);
            void closeServer();
        public:
            TcpServer(std::string ipAddress, int port);
            virtual ~TcpServer(); // Allows overide for subclasses (HttpServer)

            void runServer();
            virtual void onClient(int client);    // DOESN'T DO ANYTHING YET
    };

    class HttpResponse { // POST
        public:
            enum class StatusCodes : int { // Didn't know that you could make that corrispond to something (pretty cool ngl)
                OK = 200,
                BAD_REQUEST = 400,
                UNAUTHORIZED = 401,
                FORBIDDEN = 403,
                NOT_FOUND = 404,
                TOO_MANY_REQUESTS = 429,
                INTERNAL_SERVER_ERROR = 500
            };

            StatusCodes status;
            std::string body;
            std::string contentType;

            HttpResponse(StatusCodes status, std::string body, std::string type);
            std::string toHttpString();
        private:
            std::string statusToString(StatusCodes status);
    };

    class HttpConnection { // Convert http abstractions to a tcp usable format
        private:
            int client;

            HttpResponse::StatusCodes status;
            std::string bodyBuffer;
            std::string type;

        public:
            explicit HttpConnection(int client);
            void sendErrorNoHandler();
            void sendBuffer(); // Sends all cashed responses together
            void sendBuffer(std::string type, HttpResponse::StatusCodes status);

            void sendPlainText(HttpResponse::StatusCodes status, std::string body);
            void sendPlainText(std::string body); // Default status: 200
            void data(std::string type, HttpResponse::StatusCodes status, std::string body);
            void data(std::string type, std::string body);
    };

    class HttpServer : public TcpServer { // All the abstractions for http
        public:
            HttpServer(std::string ipAddress, int port);
            ~HttpServer();
            void run();
            void setHandler(std::function<void(HttpConnection&)> h);

            void GET(std::string endpoint, std::function<void(HttpConnection&)> h);
            void POST(std::string endpoint, std::function<void(HttpConnection&)> h);
            void PUT(std::string endpoint, std::function<void(HttpConnection&)> h);
            void DELETE(std::string endpoint, std::function<void(HttpConnection&)> h);
            void PATCH(std::string endpoint, std::function<void(HttpConnection&)> h);
            void OPTIONS(std::string endpoint, std::function<void(HttpConnection&)> h);
            void HEAD(std::string endpoint, std::function<void(HttpConnection&)> h);

        private:
            std::thread serverThread;
            std::function<void(HttpConnection&)> handler;

            struct Endpoints {
                std::string endpoint;
                std::string method;
                std::function<void(HttpConnection&)> handler;
            };
            std::vector<Endpoints> allEndpoints;

            void onClient(int client) override;
            void createEndpoint(std::string method, std::string endpoint, std::function<void(HttpConnection&)> h);
    };
}

// So you can write Status::OK instead of this
using Status = http::HttpResponse::StatusCodes;

/* 
    ##### Inspiration #####

    https://github.com/bozkurthan/Simple-TCP-Server-Client-CPP-Example/blob/master/tcp-Server.cpp
    https://www.geeksforgeeks.org/c/tcp-server-client-implementation-in-c/
    https://man7.org/linux/man-pages/man2/bind.2.html etc.

    ##### Logic-Roadmap #####

    1. HttpServer creates a TcpServer object
    2. That has a callback (onClient) that, everytime a client connects, gives back the client
    3. HttpServer registers that new connection and gives that to HttpConnection (logic)
    4. That then does all your logic and converts it to valid tcp with HttpResponse

*/