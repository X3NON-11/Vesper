#pragma once

#include <algorithm>        // std::remove_if
#include <netinet/tcp.h>    // Set timeout

#include "../utils/logging.h"        // My own logging library/header
#include "../http/radixTree.h"      // Used for the tries that saves all the endpoints and middlewares
#include "../utils/urlEncoding.h"    // Used to decode/encode url in HttpConnection
#include "../tcp/TcpServer.h"

namespace vesper {
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
                std::vector<std::function<void(HttpConnection&)>> chain = { std::forward<Handlers>(handlers)... };
                createEndpoint("GET", endpoint, [chain](HttpConnection &c){
                    size_t index = 0;
                    std::function<void()> runNext;
                    runNext = [&](){
                        if (index >= chain.size()) return;
                        auto &fn = chain[index++];
                        fn(c);
                        if (index < chain.size()) runNext();
                    };
                    runNext();
                });
            }
            template<typename... Handlers>
            void POST(std::string endpoint, Handlers&&... handlers) {
                std::vector<std::function<void(HttpConnection&)>> chain = { std::forward<Handlers>(handlers)... };
                createEndpoint("POST", endpoint, [chain](HttpConnection &c){
                    size_t index = 0;
                    std::function<void()> runNext;
                    runNext = [&](){
                        if (index >= chain.size()) return;
                        auto &fn = chain[index++];
                        fn(c);
                        if (index < chain.size()) runNext();
                    };
                    runNext();
                });
            }
            template<typename... Handlers>
            void PUT(std::string endpoint, Handlers&&... handlers) {
                std::vector<std::function<void(HttpConnection&)>> chain = { std::forward<Handlers>(handlers)... };
                createEndpoint("POST", endpoint, [chain](HttpConnection &c){
                    size_t index = 0;
                    std::function<void()> runNext;
                    runNext = [&](){
                        if (index >= chain.size()) return;
                        auto &fn = chain[index++];
                        fn(c);
                        if (index < chain.size()) runNext();
                    };
                    runNext();
                });
            }
            template<typename... Handlers>
            void DELETE(std::string endpoint, Handlers&&... handlers) {
                std::vector<std::function<void(HttpConnection&)>> chain = { std::forward<Handlers>(handlers)... };
                createEndpoint("DELETE", endpoint, [chain](HttpConnection &c){
                    size_t index = 0;
                    std::function<void()> runNext;
                    runNext = [&](){
                        if (index >= chain.size()) return;
                        auto &fn = chain[index++];
                        fn(c);
                        if (index < chain.size()) runNext();
                    };
                    runNext();
                });
            }
            template<typename... Handlers>
            void PATCH(std::string endpoint, Handlers&&... handlers) {
                std::vector<std::function<void(HttpConnection&)>> chain = { std::forward<Handlers>(handlers)... };
                createEndpoint("PATCH", endpoint, [chain](HttpConnection &c){
                    size_t index = 0;
                    std::function<void()> runNext;
                    runNext = [&](){
                        if (index >= chain.size()) return;
                        auto &fn = chain[index++];
                        fn(c);
                        if (index < chain.size()) runNext();
                    };
                    runNext();
                });
            }
            template<typename... Handlers>
            void OPTIONS(std::string endpoint, Handlers&&... handlers) {
                std::vector<std::function<void(HttpConnection&)>> chain = { std::forward<Handlers>(handlers)... };
                createEndpoint("OPTIONS", endpoint, [chain](HttpConnection &c){
                    size_t index = 0;
                    std::function<void()> runNext;
                    runNext = [&](){
                        if (index >= chain.size()) return;
                        auto &fn = chain[index++];
                        fn(c);
                        if (index < chain.size()) runNext();
                    };
                    runNext();
                });
            }
            template<typename... Handlers>
            void HEAD(std::string endpoint, Handlers&&... handlers) {
                std::vector<std::function<void(HttpConnection&)>> chain = { std::forward<Handlers>(handlers)... };
                createEndpoint("HEAD", endpoint, [chain](HttpConnection &c){
                    size_t index = 0;
                    std::function<void()> runNext;
                    runNext = [&](){
                        if (index >= chain.size()) return;
                        auto &fn = chain[index++];
                        fn(c);
                        if (index < chain.size()) runNext();
                    };
                    runNext();
                });
            }

            // Middleware
            void use(std::function<void(HttpConnection&)> handler); // Create a global middleware that runs for everything
            void setMiddleware(std::string endpoint, std::string method, std::function<void(HttpConnection&)> handler); // Create a middleware for one endpoint

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