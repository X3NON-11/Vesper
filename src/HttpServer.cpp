#include "include/radixTree.h"
#include "include/server.h"

namespace vesper { // HTTP-SERVER
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
    log(LogType::Info, "Starting Server");
    if (ipAddress == "localhost")
        ipAddress = "127.0.0.1";
    startServer(ipAddress, port);
    serverThread = std::thread(&TcpServer::runServer, this);
}

// Decides when & at what endpoint to run the handlers
void HttpServer::onClient(int client) {
    // Create the object that gets access by the library user
    HttpConnection connection(client);

    // Getting what endpoint, method client has/wants to later run the correct
    // handler Receive data from client
    const int bufferSize = 1024;
    char buffer[bufferSize] = {0};
    int valread = recv(client, buffer, bufferSize - 1, 0);
    if (valread < 0) {
        log(LogType::Warn, "Couldn't receive client data");
    }
    // Parse http data
    char method[10], clientEndpoint[256], version[10];
    if (sscanf(buffer, "%s %s %s", method, clientEndpoint, version) !=
        3) { // https://cplusplus.com/reference/cstdio/sscanf/
        log(LogType::Warn, "Failed to parse http request");
    }

    // Skip Website Logo if client connected with a browser
    if (strcmp(clientEndpoint, "/favicon.ico") == 0) {
        log(LogType::Info, "Ignoring favicon request");
        close(client);
        return;
    }

    // Parse remaining headers
    connection.clientHeaders = parseHeaders(buffer, bufferSize);

    // Get Body for POST requests
    // Skip the "\r\n\r\n"
    char *body = strstr(buffer, "\r\n\r\n");
    if (body) {
        body += 4;
    }
    int headerSize = body ? (body - buffer) : valread;
    int receivedBodySize = valread - headerSize;
    // Get the content length
    int contentLength = 0;
    char *cl = strstr(buffer, "Content-Length:");
    if (cl) {
        sscanf(cl, "Content-Length: %d", &contentLength);
    }
    // Receive the rest of the message if not fully received
    while (receivedBodySize < contentLength) {
        int r = recv(client, buffer + valread, bufferSize - valread - 1, 0);
        if (r <= 0)
            break;
        valread += r;
        receivedBodySize += r;
    }
    buffer[valread] = '\0';
    // Save the body to postData
    std::string postData = "";
    if (body && contentLength > 0) {
        postData.assign(body, contentLength);
    }

    connection.setClientBuffer(body);
    // Convert to string
    std::string header = "";
    std::string request(buffer, valread);

    // Find end of headers
    size_t headerEnd = request.find("\r\n\r\n");
    if (headerEnd != std::string::npos) {
        header = request.substr(0, headerEnd);
    }
    connection.clientHeader = header;
    connection.clientMethod = std::string(method);
    connection.clientEndpoint = clientEndpoint;
    connection.clientHttpVersion = version;
    bool handled = false;

    // Adjust clientEndpoint given to the handlers so querys are disregarded
    std::string endpointStr(clientEndpoint);
    auto pos = endpointStr.find('?');
    if (pos != std::string::npos) {
        // First get position to avoid std::out_of_range
        std::string path = endpointStr.substr(0, pos);
        std::string query = endpointStr.substr(pos + 1);

        std::snprintf(clientEndpoint, sizeof(clientEndpoint), "%s",
                      path.c_str());
        endpointStr = path;
        connection.clientQuery = decodeURL(query);
    }

    std::unordered_map<std::string, std::string> parameterMap =
        endpointsTree.getUrlParams(endpointStr, std::string(method));
    for (auto &pair : parameterMap) {
        pair.second = decodeURL(pair.second);
    }
    connection.clientParams = parameterMap;

    // MiddleWare / All Handlers
    std::vector<std::function<void(HttpConnection &)>> middlewares;
    middlewareTree.collectPrefixHandlers(endpointStr, method, middlewares);

    runMiddlewareChain(connection, middlewares, 0, [&]() {
        if (endpointsTree.matchURL(endpointStr, method)) {
            auto h = endpointsTree.getNodeHandler(endpointStr, method);
            if (h) {
                h(connection);
                handled = true;
            }
        }
    });

    if (!handled)
        connection.sendErrorNoHandler();

    connection.sendBuffer();
    close(client);

    // On the bottom so favicon.ico is skipped in the logs
    log(LogType::Info, "Client accepted");
}

// Create & Save Endpoint to allEndpoints so it is handled in onClient()
void HttpServer::createEndpoint(std::string method, std::string endpoint,
                                std::function<void(HttpConnection &)> h) {
    endpointsTree.addURL(endpoint, method, h);
    log(LogType::Info, "Succesfully created endpoint [" + endpoint + "]");
}

// Middleware
// Create a middleware by saving it to allMiddleware
// This then gets processed in runMiddlewares()
void HttpServer::setMiddleware(std::string endpoint, std::string method,
                               std::function<void(HttpConnection &)> handler) {
    endpointsTree.addURL(endpoint, method, handler);
    log(LogType::Info, "Succesfully created middleware  [" + endpoint + "]");
}
// Sets a global middleware
void HttpServer::use(std::function<void(HttpConnection &)> handler) {
    middlewareTree.addURL("/", "ALL", handler);
    log(LogType::Info, "Succesfully created middleware  [ / ]");
}

// Recursive function that goes through every Middleware to decide what to run
void HttpServer::runMiddlewareChain(
    HttpConnection &conn,
    std::vector<std::function<void(HttpConnection &)>> &mws, size_t index,
    std::function<void()> finalHandler) {
    if (index >= mws.size()) {
        finalHandler();
        return;
    }

    conn.setNext([&, index]() {
        runMiddlewareChain(conn, mws, index + 1, finalHandler);
    });

    mws[index](conn);
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