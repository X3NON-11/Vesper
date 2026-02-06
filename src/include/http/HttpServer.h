#pragma once

#include <algorithm>        // std::remove_if
#include <netinet/tcp.h>    // Set timeout

#include "../utils/logging.h"        // My own logging library/header
#include "../http/radixTree.h"      // Used for the tries that saves all the endpoints and middlewares
#include "../utils/urlEncoding.h"    // Used to decode/encode url in HttpConnection
#include "../tcp/TcpServer.h"

namespace vesper {
    class Router;
}
namespace vesper {
    // All abstractions for the httpServer itself
    // This for the library user is the server (he interacts with) & starting point of everything else
    class HttpServer : public TcpServer {
        public:
            HttpServer();
            ~HttpServer();
            void run(std::string ipAddress, int port); // Runs startServer & runServer on a different thread
            // Groups together endpoints/middleware
            vesper::Router group(std::string endpoint);

            // Abstractions to create different endpoints (runs createEndpoint())
            template<typename... Handlers>
            void GET(std::string endpoint, Handlers&&... handlers) {
                std::vector<std::function<void(HttpConnection&)>> chain = { std::forward<Handlers>(handlers)...};
                for (int i = 0; i + 1 < chain.size(); i++) {
                    setMiddleware(endpoint, "GET", true, chain[i]);
                }
                createEndpoint("GET", endpoint, chain.back());
            }
            template<typename... Handlers>
            void POST(std::string endpoint, Handlers&&... handlers) {
                std::vector<std::function<void(HttpConnection&)>> chain = { std::forward<Handlers>(handlers)...};
                for (int i = 0; i + 1 < chain.size(); i++) {
                    setMiddleware(endpoint, "POST", true,  chain[i]);
                }
                createEndpoint("POST", endpoint, chain.back());
            }
            template<typename... Handlers>
            void PUT(std::string endpoint, Handlers&&... handlers) {
                std::vector<std::function<void(HttpConnection&)>> chain = { std::forward<Handlers>(handlers)...};
                for (int i = 0; i + 1 < chain.size(); i++) {
                    setMiddleware(endpoint, "PUT", true,  chain[i]);
                }
                createEndpoint("PUT", endpoint, chain.back());
            }
            template<typename... Handlers>
            void DELETE(std::string endpoint, Handlers&&... handlers) {
                std::vector<std::function<void(HttpConnection&)>> chain = { std::forward<Handlers>(handlers)...};
                for (int i = 0; i + 1 < chain.size(); i++) {
                    setMiddleware(endpoint, "DELETE", true,  chain[i]);
                }
                createEndpoint("DELETE", endpoint, chain.back());
            }
            template<typename... Handlers>
            void PATCH(std::string endpoint, Handlers&&... handlers) {
                std::vector<std::function<void(HttpConnection&)>> chain = { std::forward<Handlers>(handlers)...};
                for (int i = 0; i + 1 < chain.size(); i++) {
                    setMiddleware(endpoint, "PATCH", true,  chain[i]);
                }
                createEndpoint("PATCH", endpoint, chain.back());
            }
            template<typename... Handlers>
            void OPTIONS(std::string endpoint, Handlers&&... handlers) {
                std::vector<std::function<void(HttpConnection&)>> chain = { std::forward<Handlers>(handlers)...};
                for (int i = 0; i + 1 < chain.size(); i++) {
                    setMiddleware(endpoint, "OPTIONS", true,  chain[i]);
                }
                createEndpoint("OPTIONS", endpoint, chain.back());
            }
            template<typename... Handlers>
            void HEAD(std::string endpoint, Handlers&&... handlers) {
                std::vector<std::function<void(HttpConnection&)>> chain = { std::forward<Handlers>(handlers)...};
                for (int i = 0; i + 1 < chain.size(); i++) {
                    setMiddleware(endpoint, "HEAD", true,  chain[i]);
                }
                createEndpoint("HEAD", endpoint, chain.back());
            }

            // Middleware
            void use(std::function<void(HttpConnection&)> handler); // Create a global middleware that runs for everything
           void setMiddleware(std::string endpoint, std::string method, bool prefix, std::function<void(HttpConnection &)> handler); // Create a middleware for one endpoint

        private:
            std::thread serverThread; // The thread the server/socket uses
            Tree endpointsTree;
            Tree middlewareTree;
            
            // Recursive function which loops over every middleware in an onion style
            void runMiddlewareChain(HttpConnection &conn,
                        std::vector<std::function<void(HttpConnection &)>> &mws,
                        size_t index, std::function<void()> finalHandler);

            // Overrides the onClient() from TcpServer
            // Decides on what endpoint & when to run what handler/middleware
            void onClient(int client) override;
            // Used to create endpoints by functions like GET()
            void createEndpoint(std::string method, std::string endpoint, std::function<void(HttpConnection&)> h);
            std::unordered_map<std::string, std::string> parseHeaders(const char* buffer, int headerSize);
    };
}

namespace vesper {
    class Router {
        private:
            vesper::HttpServer& server;
            std::string prefix;
            std::vector<std::function<void(HttpConnection&)>> middlewares;
            std::string validatePath(std::string endpoint);
            
        public:
            Router(vesper::HttpServer& server, std::string prefix);
        
            void use(std::function<void(HttpConnection&)> mw);
        
            // Abstractions to create different endpoints (runs createEndpoint())
            // DONT USE THE PREFIX AND CHANGE TO A FULL PATH
            template<typename... Handlers>
            void GET(std::string endpoint, Handlers&&... handlers) {
                std::string validatedPath = validatePath(endpoint);
                // apply group middleware first
                for (auto& mw : middlewares) {
                    server.setMiddleware(validatedPath, "GET", true,  mw);
                }
                // forward handlers
                server.GET(validatedPath, std::forward<Handlers>(handlers)...);
            }
            template<typename... Handlers>
            void POST(std::string endpoint, Handlers&&... handlers) {
                std::string validatedPath = validatePath(endpoint);
                // apply group middleware first
                for (auto& mw : middlewares) {
                    server.setMiddleware(validatedPath, "POST", true,  mw);
                }
                // forward handlers
                server.POST(validatedPath, std::forward<Handlers>(handlers)...);
            }
            template<typename... Handlers>
            void PUT(std::string endpoint, Handlers&&... handlers) {
                std::string validatedPath = validatePath(endpoint);
                // apply group middleware first
                for (auto& mw : middlewares) {
                    server.setMiddleware(validatedPath, "PUT", true,  mw);
                }
                // forward handlers
                server.PUT(validatedPath, std::forward<Handlers>(handlers)...);
            }
            template<typename... Handlers>
            void DELETE(std::string endpoint, Handlers&&... handlers) {
                std::string validatedPath = validatePath(endpoint);
                // apply group middleware first
                for (auto& mw : middlewares) {
                    server.setMiddleware(validatedPath, "DELETE", true,  mw);
                }
                // forward handlers
                server.DELETE(validatedPath, std::forward<Handlers>(handlers)...);
            }
            template<typename... Handlers>
            void PATCH(std::string endpoint, Handlers&&... handlers) {
                std::string validatedPath = validatePath(endpoint);
                // apply group middleware first
                for (auto& mw : middlewares) {
                    server.setMiddleware(validatedPath, "PATCH", true,  mw);
                }
                // forward handlers
                server.PATCH(validatedPath, std::forward<Handlers>(handlers)...);
            }
            template<typename... Handlers>
            void OPTIONS(std::string endpoint, Handlers&&... handlers) {
                std::string validatedPath = validatePath(endpoint);
                // apply group middleware first
                for (auto& mw : middlewares) {
                    server.setMiddleware(validatedPath, "OPTIONS", true,  mw);
                }
                // forward handlers
                server.OPTIONS(validatedPath, std::forward<Handlers>(handlers)...);
            }
            template<typename... Handlers>
            void HEAD(std::string endpoint, Handlers&&... handlers) {
                std::string validatedPath = validatePath(endpoint);
                // apply group middleware first
                for (auto& mw : middlewares) {
                    server.setMiddleware(validatedPath, "HEAD", true,  mw);
                }
                // forward handlers
                server.HEAD(validatedPath, std::forward<Handlers>(handlers)...);
            }
        };
}