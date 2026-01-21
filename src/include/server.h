#pragma once

#include <cstdio>           // sscanf
#include <thread>           // multithreading for different clients
#include <iostream>         // idk
#include <cstring>          // memset
#include <unistd.h>         // close
#include <sys/socket.h>     // socket, bind, listen
#include <netinet/in.h>     // sockaddr_in
#include <arpa/inet.h>      // htons, inet_aton
#include <functional>       // std::function

#include "logging.h"

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

            HttpResponse(StatusCodes status, std::string body);
            std::string toHttpString();
        private:
            std::string statusToString(StatusCodes status);
    };

    class HttpConnection { // Convert http abstractions to a tcp usable format
        private:
            int client;

        public:
            explicit HttpConnection(int client);
            void process();

            void sendPlainText(HttpResponse::StatusCodes status, std::string body);
            void sendPlainText(std::string body); // Default status: 200
    };

    class HttpServer : public TcpServer { // All the abstractions for http
        private:
            std::thread serverThread;
            std::function<void(HttpConnection&)> handler;

            void onClient(int client) override;

        public:
            HttpServer(std::string ipAddress, int port);
            ~HttpServer();
            void run();
            void setHandler(std::function<void(HttpConnection&)> h);
    };
}

/*
    ##### Logic-Roadmap #####

    1. HttpServer creates a TcpServer object
    2. That has a callback (onClient) that, everytime a client connects, gives back the client
    3. HttpServer registers that new connection and gives that to HttpConnection (logic)
    4. That then does all your logic and converts it to valid tcp with HttpResponse

*/