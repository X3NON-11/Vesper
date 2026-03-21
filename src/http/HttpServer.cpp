#include "../include/vesper/http/HttpServer.h"
#include "../include/vesper/http/radixTree.h"
#include "../include/vesper/tcp/TcpServer.h"

namespace vesper {
// ===========
// HTTP-SERVER
// ===========

HttpServer::HttpServer() : TcpServer() {
    use(loggingMiddleware, recoveryMiddleware);
}
HttpServer::HttpServer(vesper::HttpServerTypes constructor) : TcpServer() {
    if (constructor == vesper::HttpServerTypes::Default)
        use(loggingMiddleware, recoveryMiddleware);
}

// Close server automatically
HttpServer::~HttpServer() {
    if (serverThread.joinable()) {
        serverThread.join();
    }
}

// Sets up & runs the server using the previously created objects
void HttpServer::run(std::string ipAddress, int port) {
    if (ipAddress == "localhost")
        ipAddress = "127.0.0.1";
    domain = ipAddress;
    startServer(domain, port);
    serverThread = std::thread(&TcpServer::runServer, this);
    async::EventLoop::instance().loop();
}

// Decides when & at what endpoint to run the handlers
async::Task HttpServer::onClient(int client) {
    // Create the object that gets access by the library user
    HttpConnection connection(client, this);
    Context ctx{};
    ctx.buffer.resize(4096);

    // Getting what endpoint, method client has/wants to later run the
    // correct handler Receive data from client;
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
    connection.domain = domain;

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
    // logConnection(static_cast<int>(connection.response.status),
    //               connection.response.method, connection.request.path);
}

// Header Parsing
std::unordered_map<std::string, std::string>
HttpServer::parseHeaders(const char *buffer, int headerSize) {
    std::unordered_map<std::string, std::string> receivedHeaders;
    receivedHeaders.reserve(10);

    std::string_view headers(buffer, headerSize);

    int start = 0;
    while (start < headers.size()) {
        int end = headers.find('\n', start);
        if (end == std::string_view::npos)
            break;

        std::string_view line = headers.substr(start, end - start);

        if (!line.empty() && line.back() == '\r')
            line.remove_suffix(1);

        if (line.empty())
            break;

        // Split key and value
        int colon = line.find(':');
        if (colon == std::string_view::npos) {
            start = end + 1;
            continue;
        }

        std::string_view key = line.substr(0, colon);
        std::string_view value = line.substr(colon + 1);

        // Erase from 0 to the first non space/tab character
        int first = value.find_first_not_of(" \t");
        if (first != std::string_view::npos)
            value.remove_prefix(first);

        receivedHeaders.emplace(std::string(key), std::string(value));

        start = end + 1;
    }

    return receivedHeaders;
}

// =============
// onClient over
// =============

vesper::Router HttpServer::group(std::string endpoint) {
    return vesper::Router(*this, endpoint);
}

void HttpServer::staticDir(std::string endpoint, std::string folder) {

    if (!std::filesystem::exists(folder) ||
        !std::filesystem::is_directory(folder)) {
        log(LogType::Warn,
            "Directory \"" + folder + "\" doesn't exist or is not a directory");
        return;
    }

    for (const auto &entry : std::filesystem::directory_iterator(folder)) {
        if (!entry.is_regular_file())
            continue;

        std::string filePath = entry.path().string();

        std::string relativePath = entry.path().filename().string();

        std::string fileEndpoint = endpoint;

        if (!fileEndpoint.empty() && fileEndpoint.back() != '/')
            fileEndpoint += '/';

        fileEndpoint += relativePath;

        staticFile(fileEndpoint, filePath);
    }
}

void HttpServer::staticFile(std::string endpoint, std::string file) {
    if (!std::filesystem::exists(file)) {
        log(LogType::Warn, "File \"" + file + "\" doesn't exist");
        return;
    }

    std::ifstream f(file, std::ios::binary);
    if (!f) {
        log(LogType::Warn, "Couldn't open file \"" + file + "\"");
        return;
    }

    // Read file fully
    std::string content((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());

    staticFileInfo sf;
    sf.content = zlibCompress(content);
    sf.contentType = getMimeType(file);
    staticFilesMap[endpoint] = sf;

    auto handler = [this](vesper::HttpConnection &c) {
        if (c.request.method == "GET" || c.request.method == "HEAD") {
            c.header("X-Content-Type-Options", "nosniff");
            c.header("Content-Security-Policy", "default-src 'self';");
            c.header("X-Frame-Options", "DENY");
            c.header("Content-Encoding", "deflate");
            staticFileInfo file = staticFilesMap[c.request.path];
            c.data(file.contentType, file.content);
        } else {
            c.string("");
        }
    };
    endpointsTree.addURL(endpoint, "GET", false, handler);
}

std::string HttpServer::getMimeType(std::string file) {
    static const std::unordered_map<std::string, std::string> mimeMap = {
        // Text
        {"txt", "text/plain"},
        {"html", "text/html"},
        {"htm", "text/html"},
        {"css", "text/css"},
        {"csv", "text/csv"},
        {"js", "application/javascript"},
        {"json", "application/json"},
        {"xml", "application/xml"},

        // Images
        {"png", "image/png"},
        {"jpg", "image/jpeg"},
        {"jpeg", "image/jpeg"},
        {"gif", "image/gif"},
        {"bmp", "image/bmp"},
        {"webp", "image/webp"},
        {"svg", "image/svg+xml"},
        {"ico", "image/x-icon"},

        // Fonts
        {"ttf", "font/ttf"},
        {"otf", "font/otf"},
        {"woff", "font/woff"},
        {"woff2", "font/woff2"},

        // Documents
        {"pdf", "application/pdf"},
        {"zip", "application/zip"},
        {"gz", "application/gzip"},
        {"tar", "application/x-tar"},

        // Media
        {"mp3", "audio/mpeg"},
        {"wav", "audio/wav"},
        {"mp4", "video/mp4"},
        {"webm", "video/webm"},
        {"ogg", "audio/ogg"},
        {"ogv", "video/ogg"}};

    size_t dotPos = file.find_last_of('.');
    if (dotPos == std::string::npos || dotPos + 1 >= file.size()) {
        log(LogType::Warn, "Invalid file");
        return "application/octet-stream";
    }

    std::string extension = file.substr(dotPos + 1);

    auto it = mimeMap.find(extension);
    if (it != mimeMap.end()) {
        return it->second;
    }

    // Safe fallback
    return "application/octet-stream";
}

std::string HttpServer::zlibCompress(std::string &input) {
    uLong sourceLen = input.size();
    uLong destLen = compressBound(sourceLen);
    std::vector<unsigned char> buffer(destLen);

    int res = compress2(buffer.data(), &destLen,
                        reinterpret_cast<const Bytef *>(input.data()),
                        sourceLen, Z_BEST_COMPRESSION);

    if (res != Z_OK)
        return {};

    return std::string(reinterpret_cast<char *>(buffer.data()), destLen);
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
} // namespace vesper

namespace vesper {
// Responsible for server.group()
// Works by reusing HttpServer() functions just with the correct prefix
Router::Router(HttpServer &server, std::string prefix)
    : server(server), prefix(prefix) {
    log(LogType::Info, "ALL " + prefix);
}

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