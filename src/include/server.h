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
    // The foundation of the program
    // This handles the basic socket
    class TcpServer {
        protected: // Allows acces for subclasses
            int listenSocket; // Socket that listens for new connections
            int port; // The port the listenSocket runs on

        public:
            TcpServer() = default; // Only creates a object
            virtual ~TcpServer(); // Allows overide for subclasses (HttpServer)

            // Listens for new connections (clients)
            void runServer(); 
            // Logic on connection (overwritten in HttpServer)
            virtual void onClient(int client); 

            // sets everything up for a basic Tcp Server
            int startServer(std::string ipAddress, int port);
            // closes socket (is run in the TcpServer destructor)
            void closeServer(); 
    };

    // Conveniently set up an object with every desired parameter and then convert it to an http 1.1 string
    class HttpResponse {
        public:
            // Map every StatusCode (e.g can be accessed as Status::OK)
            enum class StatusCodes : int { 
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

    // Stores all the http abstractions / what you would access as a library user (e.g c.string("Hello World"); )
    class HttpConnection {
        private:
            int client;

            HttpResponse::StatusCodes responseStatus;
            std::string bodyBuffer;
            std::string type;
            std::function<void()> nextFn;

        public:
            explicit HttpConnection(int client);
            void sendErrorNoHandler();
            void sendBuffer(); // Sends all cashed responses together
            void sendBuffer(std::string type, HttpResponse::StatusCodes status);

            // Abstractions for convenience (only calls data())
            void string(HttpResponse::StatusCodes status, std::string body);
            void string(std::string body); // Default status: 200
            void json(HttpResponse::StatusCodes status, std::string jsonBody);
            void json(std::string jsonBody); // Default status: 200
            void status(HttpResponse::StatusCodes status);

            // Handles/Sends every supported data type by storing it correctly in the bodyBuffer
            void data(std::string type, HttpResponse::StatusCodes status, std::string body);
            void data(std::string type, std::string body);

            // Used in middleware to stop at this point, execute next middleware, then proceed
            void next();
            // Used to set the next middleware for the HttpConnection from HttpServer
            void setNext(std::function<void()> fn);
    };

    // All abstractions for the httpServer itself
    // This for the library user is the server (he interacts with) & starting point of everything else
    class HttpServer : public TcpServer {
        public:
            HttpServer();
            ~HttpServer();
            void run(std::string ipAddress, int port); // Runs startServer & runServer on a different thread

            // Abstractions to create different endpoints (runs createEndpoint())
            template<typename... Handlers>
            void GET(std::string endpoint, Handlers&&... handlers) {
                // Combine all handlers into one vector
                std::vector<std::function<void(HttpConnection&)>> chain = { std::forward<Handlers>(handlers)... };
                int index = chain.size();
                for (int i = 0; i < index-1; i++) {
                    setMiddleware(endpoint, "GET", chain[i]);
                }
                createEndpoint("GET", endpoint, chain[index-1]);
            }
            template<typename... Handlers>
            void POST(std::string endpoint, Handlers&&... handlers) {
                // Combine all handlers into one vector
                std::vector<std::function<void(HttpConnection&)>> chain = { std::forward<Handlers>(handlers)... };
                int index = chain.size();
                for (int i = 0; i < index-1; i++) {
                    setMiddleware(endpoint, "POST", chain[i]);
                }
                createEndpoint("POST", endpoint, chain[index-1]);
            }
            template<typename... Handlers>
            void PUT(std::string endpoint, Handlers&&... handlers) {
                // Combine all handlers into one vector
                std::vector<std::function<void(HttpConnection&)>> chain = { std::forward<Handlers>(handlers)... };
                int index = chain.size();
                for (int i = 0; i < index-1; i++) {
                    setMiddleware(endpoint, "PUT", chain[i]);
                }
                createEndpoint("PUT", endpoint, chain[index-1]);
            }
            template<typename... Handlers>
            void DELETE(std::string endpoint, Handlers&&... handlers) {
                // Combine all handlers into one vector
                std::vector<std::function<void(HttpConnection&)>> chain = { std::forward<Handlers>(handlers)... };
                int index = chain.size();
                for (int i = 0; i < index-1; i++) {
                    setMiddleware(endpoint, "DELETE", chain[i]);
                }
                createEndpoint("DELETE", endpoint, chain[index-1]);
            }
            template<typename... Handlers>
            void PATCH(std::string endpoint, Handlers&&... handlers) {
                // Combine all handlers into one vector
                std::vector<std::function<void(HttpConnection&)>> chain = { std::forward<Handlers>(handlers)... };
                int index = chain.size();
                for (int i = 0; i < index-1; i++) {
                    setMiddleware(endpoint, "PATCH", chain[i]);
                }
                createEndpoint("PATCH", endpoint, chain[index-1]);
            }
            template<typename... Handlers>
            void OPTIONS(std::string endpoint, Handlers&&... handlers) {
                // Combine all handlers into one vector
                std::vector<std::function<void(HttpConnection&)>> chain = { std::forward<Handlers>(handlers)... };
                int index = chain.size();
                for (int i = 0; i < index-1; i++) {
                    setMiddleware(endpoint, "OPTIONS", chain[i]);
                }
                createEndpoint("OPTIONS", endpoint, chain[index-1]);
            }
            template<typename... Handlers>
            void HEAD(std::string endpoint, Handlers&&... handlers) {
                // Combine all handlers into one vector
                std::vector<std::function<void(HttpConnection&)>> chain = { std::forward<Handlers>(handlers)... };
                int index = chain.size();
                for (int i = 0; i < index-1; i++) {
                    setMiddleware(endpoint, "HEAD", chain[i]);
                }
                createEndpoint("HEAD", endpoint, chain[index-1]);
            }

            // Middleware
            void use(std::function<void(HttpConnection&)> handler); // Create a global middleware that runs for everything
            void setMiddleware(std::string endpoint, std::string method, std::function<void(HttpConnection&)> handler); // Create a middleware for one endpoint

        private:
            std::thread serverThread; // The thread the server/socket uses

            // Store all Endpoints which are used in onClient()
            struct Endpoints {
                std::string endpoint;
                std::string method;
                std::function<void(HttpConnection&)> handler;
            };
            std::vector<Endpoints> allEndpoints;

            // Store every Middleware which is used in onClient()/runMiddleware()
            struct Middleware {
                std::string endpoint;
                std::string method;
                std::function<void(HttpConnection&)> handler;
                std::function<void(HttpConnection&)> nextHandler;
            };
            std::vector<Middleware> allMiddleware;
            // Recursive function which loops over every middleware in an onion style
            void runMiddlewares(HttpConnection& connection, std::string clientEndpoint, std::string method, int index, std::function<void()> finalHandler);

            // Overrides the onClient() from TcpServer
            // Decides on what endpoint & when to run what handler/middleware
            void onClient(int client) override;
            // Used to create endpoints by functions like GET()
            void createEndpoint(std::string method, std::string endpoint, std::function<void(HttpConnection&)> h);
    };
}

// So you can write Status::OK instead of this
using Status = http::HttpResponse::StatusCodes;