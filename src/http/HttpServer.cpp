#include "../include/http/HttpServer.h"
#include "../include/http/radixTree.h"
#include "../include/tcp/TcpServer.h"

namespace vesper {
// ===========
// HTTP-SERVER
// ===========

HttpServer::HttpServer() : TcpServer() {}

// Close server automatically
HttpServer::~HttpServer() {
    if (serverThread.joinable()) {
        serverThread.join();
    }
}

// Sets up & runs the server using the previously created objects
void HttpServer::run(std::string ipAddress, int port) {
    setupLogger();
    if (ipAddress == "localhost")
        ipAddress = "127.0.0.1";
    startServer(ipAddress, port);
    serverThread = std::thread(&TcpServer::runServer, this);
    async::EventLoop::instance().loop();
}

// Decides when & at what endpoint to run the handlers
async::Task HttpServer::onClient(int client) {
    // Create the object that gets access by the library user
    HttpConnection connection(client, this);
    Context ctx{};
    ctx.buffer.resize(4096);

    // Getting what endpoint, method client has/wants to later run the correct
    // handler Receive data from client;
    while (ctx.request.find("\r\n\r\n") == std::string::npos) {
        // int r = recv(client, buffer.data(), buffer.size(), 0);
        int r = co_await async::RecvAwaiter{client, ctx.buffer.data(),
                                            ctx.buffer.size()};
        if (r > 0) {
            ctx.request.append(ctx.buffer.data(), r);
        } else if (r == 0) {
            log(LogType::Warn, "Client closed connection");
            close(client);
            co_return;
        } else { // r < 0
            log(LogType::Warn, "recv error");
            co_return;
        }

        // Prevent header abuse
        if (ctx.request.size() > 16 * 1024) {
            log(LogType::Warn, "HTTP headers too large");
            close(client);
            co_return;
        }
    }

    // Parse http header
    // https://cplusplus.com/reference/cstdio/sscanf/
    if (sscanf(ctx.request.data(), "%s %s %s", ctx.method, ctx.clientEndpoint,
               ctx.version) != 3) {
        log(LogType::Warn, "Failed to parse http header");
        close(client);
        co_return;
    }

    // Skip Website Logo if client connected with a browser
    if (strcmp(ctx.clientEndpoint, "/favicon.ico") == 0) {
        close(client);
        co_return;
    }

    // Parse remaining headers
    connection.request.headers =
        parseHeaders(ctx.request.data(), ctx.request.size());

    // Get the content length
    ctx.contentLength = 0;
    char *cl = strstr(ctx.request.data(), "Content-Length:");
    if (cl)
        sscanf(cl, "Content-Length: %d", &ctx.contentLength);

    // Find end of headers
    ctx.headerEnd = ctx.request.find("\r\n\r\n");
    if (ctx.headerEnd != std::string::npos &&
        ctx.headerEnd + 4 < ctx.request.size()) {
        ctx.body = ctx.request.substr(ctx.headerEnd + 4);
    }

    // Save the body to postData
    ctx.postData = ctx.body;
    auto start = std::chrono::steady_clock::now();
    while (ctx.postData.size() < ctx.contentLength) {
        int r = co_await async::RecvAwaiter{client, ctx.buffer.data(),
                                            ctx.buffer.size()};
        if (r > 0) {
            ctx.postData.append(ctx.buffer.data(), r);
        } else if (r == 0) {
            log(LogType::Warn, "Client closed during body");
            close(client);
            co_return;
        } else {
            log(LogType::Warn, "recv error");
            co_return;
        }

        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - start)
                .count() > timeout) {
            log(LogType::Warn, "Client took too long to send body");
            close(client);
            co_return;
        }
    }

    // Saves all the variables to connection
    connection.setClientBuffer(ctx.postData);
    connection.request.method = std::string(ctx.method);
    connection.request.path = ctx.clientEndpoint;
    connection.request.httpVersion = ctx.version;

    // Adjust clientEndpoint given to the handlers so querys are
    // disregarded
    ctx.endpointStr = std::string(ctx.clientEndpoint);
    auto pos = ctx.endpointStr.find('?');
    if (pos != std::string::npos) {
        std::string path = ctx.endpointStr.substr(0, pos);
        std::string query = ctx.endpointStr.substr(pos + 1);

        std::snprintf(ctx.clientEndpoint, sizeof(ctx.clientEndpoint), "%s",
                      path.c_str());
        ctx.endpointStr = path;
        connection.request.rawQuery = decodeURL(query);
    }

    // MiddleWare / All Handlers
    std::unordered_map<std::string, std::string> parameterMap =
        endpointsTree.getUrlParams(ctx.endpointStr, std::string(ctx.method));
    for (auto &pair : parameterMap) {
        pair.second = decodeURL(pair.second);
    }
    connection.request.params = parameterMap;

    // Run Middlewares and handler
    // Runs mutable so when the function ends the thread can still execute
    // everything, because it is copied
    threads.newTask([this, client, connection, ctx]() mutable {
        handleRequest(client, connection, ctx);
    });

    co_return;
}

void HttpServer::handleRequest(int client, HttpConnection &connection,
                               Context &ctx) {

    std::vector<std::function<void(HttpConnection &)>> middlewares;
    middlewareTree.collectPrefixHandlers(ctx.endpointStr, ctx.method,
                                         middlewares);

    bool handled = false;

    runMiddlewareChain(connection, middlewares, 0, [&]() {
        if (endpointsTree.matchURL(ctx.endpointStr, ctx.method)) {
            auto h = endpointsTree.getNodeHandler(ctx.endpointStr, ctx.method);
            if (h) {
                h(connection);
                handled = true;
            }
        }
    });

    if (!handled)
        connection.string(404, "");

    // Close connection and logs
    std::string http = connection.response.toHttpString();
    send(client, http.c_str(), http.size(), 0);
    close(client);
    logConnection(static_cast<int>(connection.response.status),
                  connection.response.method, connection.request.path);
}

// =============
// onClient over
// =============

vesper::Router HttpServer::group(std::string endpoint) {
    return vesper::Router(*this, endpoint);
}
// Endpoint
// Create & Save Endpoint to allEndpoints so it is handled in
// onClient()
void HttpServer::createEndpoint(std::string method, std::string endpoint,
                                std::function<void(HttpConnection &)> h) {
    endpointsTree.addURL(endpoint, method, false, h);
    log(LogType::Info, method + " " + endpoint);
}

// Middleware
// Create a middleware by saving it to allMiddleware
// This then gets processed in runMiddlewares()
void HttpServer::setMiddleware(std::string endpoint, std::string method,
                               bool prefix,
                               std::function<void(HttpConnection &)> handler) {
    middlewareTree.addURL(endpoint, method, prefix, handler);
    log(LogType::Info, method + " " + endpoint);
}

// Recursive function that goes through every Middleware to decide
// what to run
void HttpServer::runMiddlewareChain(
    HttpConnection &c, std::vector<std::function<void(HttpConnection &)>> &mws,
    size_t index, std::function<void()> finalHandler) {
    if (index >= mws.size()) {
        // threads.newTask([finalHandler]() { finalHandler(); });
        finalHandler();
        return;
    }

    bool nextCalled = false;

    c.setNext([&]() {
        nextCalled = true;
        runMiddlewareChain(c, mws, index + 1, finalHandler);
    });

    mws[index](c);

    // Stop chain if middleware didn't call next()
    if (!nextCalled) {
        return;
    }
}

// Header Parsing
std::unordered_map<std::string, std::string>
HttpServer::parseHeaders(const char *buffer, int headerSize) {
    std::unordered_map<std::string, std::string> receivedHeaders;
    std::string headerStr(buffer, headerSize);
    // https://stackoverflow.com/questions/12514510/iterate-through-lines-in-a-string-c
    std::istringstream stream(headerStr);
    std::string line;

    while (std::getline(stream, line)) {
        // Remove new line character at the end of the line
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (line.empty())
            break;

        // Split key and value
        int colon = line.find(':');
        if (colon == std::string::npos)
            continue;

        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);

        // Remove white spaces
        // https://stackoverflow.com/questions/83439/remove-spaces-from-stdstring-in-c
        key.erase(std::remove_if(key.begin(), key.end(), ::isspace), key.end());
        // Erase from 0 to the first non space/tab character
        value.erase(0, value.find_first_not_of(" \t"));

        receivedHeaders[key] = value;
    }

    return receivedHeaders;
}
} // namespace vesper

namespace vesper {
// Responsible for server.group()
// Works by reusing HttpServer() functions just with the correct prefix
Router::Router(HttpServer &server, std::string prefix)
    : server(server), prefix(prefix) {}

std::string Router::validatePath(std::string endpoint) {
    std::string path = endpoint;
    // ensure leading slash
    if (path[0] != '/')
        path = '/' + path;

    if (!prefix.empty()) {
        std::string fullPath = prefix;
        if (fullPath.back() == '/')
            fullPath.pop_back(); // remove trailing slash
        path = fullPath + path;
    }
    return path;
}
} // namespace vesper