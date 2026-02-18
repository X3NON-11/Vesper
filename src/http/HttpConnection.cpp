#include "../include/http/HttpConnection.h"
#include "../include/http/HttpServer.h" // In cpp to avoid circular include

namespace vesper {
// =====================================================
// HTTP-RESPONSE used to convert http logic to tcp logic
// =====================================================

HttpResponse::HttpResponse(StatusCodes status, std::string body,
                           std::string type, std::string method)
    : status(status), body(body), method(method) {
    if (type == "text/plain") {
        contentType = "Content-Type: text/plain; charset=utf-8";
    } else if (type == "text/html") {
        contentType = "Content-Type: text/html; charset=utf-8";
    } else if (type == "text/css") {
        contentType = "Content-Type: text/css; charset=utf-8";
    } else if (type == "application/javascript") {
        contentType = "Content-Type: application/javascript; charset=utf-8";
    } else if (type == "application/json") {
        contentType = "Content-Type: application/json; charset=utf-8";
    } else if (type == "application/xml") {
        contentType = "Content-Type: application/xml; charset=utf-8";
    } else if (type == "image/png") {
        contentType = "Content-Type: image/png";
    } else if (type == "image/jpeg") {
        contentType = "Content-Type: image/jpeg";
    } else if (type == "image/gif") {
        contentType = "Content-Type: image/gif";
    } else if (type == "application/pdf") {
        contentType = "Content-Type: application/pdf";
    } else {
        contentType = "Content-Type: application/octet-stream";
    }
}

std::string HttpResponse::toHttpString() {
    std::string response;

    // Status line
    response += "HTTP/1.1 " + std::to_string(static_cast<int>(status)) + " " +
                statusToString(status) + "\r\n";

    // Required / default headers
    response += "Content-Type: " + contentType + "\r\n";
    response += "Content-Length: " + std::to_string(body.size()) + "\r\n";

    // Custom headers
    for (const auto &[key, value] : headers) {
        response += key + ": " + value + "\r\n";
    }

    // End of headers
    response += "\r\n";

    // Body
    response += body;

    return response;
}

std::string HttpResponse::statusToString(StatusCodes status) {
    switch (status) {
    case StatusCodes::OK:
        return "OK";
    case StatusCodes::BAD_REQUEST:
        return "Bad Request";
    case StatusCodes::UNAUTHORIZED:
        return "Unauthorized";
    case StatusCodes::FORBIDDEN:
        return "Forbidden";
    case StatusCodes::NOT_FOUND:
        return "Not Found";
    case StatusCodes::TOO_MANY_REQUESTS:
        return "Too Many Requests";
    case StatusCodes::INTERNAL_SERVER_ERROR:
        return "Internal Server Error";
    default:
        return "Unknown";
    }
}

void HttpResponse::setHeader(const std::string &name,
                             const std::string &value) {
    headers[name] = value;
}

void HttpResponse::removeHeader(std::string &name) { headers.erase(name); }

// =============================================================================
// HTTP-CONNECTION responsible for translating abstractions to tcp usable format
// =============================================================================

HttpConnection::HttpConnection(int client, vesper::HttpServer *server)
    : client(client), server(server) {}

// All abstractions like c.string to send plain text
void HttpConnection::sendErrorNoHandler() {
    string(HttpResponse::StatusCodes::INTERNAL_SERVER_ERROR,
           "No handler parsed");
    log(LogType::Warn, "No handler parsed");
}

void HttpConnection::string(int status, std::string body) {
    if (response.body != "")
        log(LogType::Warn, "Only one message is intended - the first content "
                           "header applies to both");
    response.body += body;
    response.contentType = "text/plain";
    response.status = static_cast<HttpResponse::StatusCodes>(status);
}
void HttpConnection::string(HttpResponse::StatusCodes status,
                            std::string body) {
    if (response.body != "")
        log(LogType::Warn, "Only one message is intended - the first content "
                           "header applies to both");
    response.body += body;
    response.contentType = "text/plain";
    response.status = status;
}
void HttpConnection::string(std::string body) {
    string(HttpResponse::StatusCodes::OK, body);
}

void HttpConnection::json(int status, std::string jsonBody) {
    if (response.body != "")
        log(LogType::Warn, "Only one message is intended - the first content "
                           "header applies to both");
    response.body += jsonBody;
    response.contentType = "application/json";
    response.status = static_cast<HttpResponse::StatusCodes>(status);
}
void HttpConnection::json(HttpResponse::StatusCodes status,
                          std::string jsonBody) {
    if (response.body != "")
        log(LogType::Warn, "Only one message is intended - the first content "
                           "header applies to both");
    response.body += jsonBody;
    response.contentType = "application/json";
    response.status = status;
}
void HttpConnection::json(std::string jsonBody) {
    json(HttpResponse::StatusCodes::OK, jsonBody);
}

void HttpConnection::status(int status) {
    response.status = static_cast<HttpResponse::StatusCodes>(status);
}
void HttpConnection::status(HttpResponse::StatusCodes status) {
    response.status = status;
}

void HttpConnection::data(std::string type, int status, std::string body) {
    if (response.body != "")
        log(LogType::Warn, "Only one message is intended - the first content "
                           "header applies to both");
    response.body += body;
    response.contentType = type;
    response.status = static_cast<HttpResponse::StatusCodes>(status);
}
void HttpConnection::data(std::string type, HttpResponse::StatusCodes status,
                          std::string body) {
    if (response.body != "")
        log(LogType::Warn, "Only one message is intended - the first content "
                           "header applies to both");
    response.body += body;
    response.contentType = type;
    response.status = status;
}
void HttpConnection::data(std::string type, std::string body) {
    data(type, HttpResponse::StatusCodes::OK, body);
}

void HttpConnection::header(std::string hName, std::string hContent) {
    response.headers[hName] = hContent;
}

// Middleware
// Used to set the next middleware for the HttpConnection from HttpServer
void HttpConnection::setNext(std::function<void()> fn) { nextFn = fn; }

// Used in middleware to stop at this point, execute next middleware, then
// proceed
void HttpConnection::next() {
    if (nextFn) {
        nextFn();
        nextFn = nullptr;
    }
}

// Receive client data (POST etc.)
std::string HttpConnection::defaultPostForm(std::string clientString,
                                            std::string defaultString) {
    if (request.method == "GET") {
        return defaultString;
    }

    // Loop through every argument by looking for '&' (except the last argument
    // because there is no '&')
    int start = 0;
    int end;
    while ((end = request.body.find('&', start)) != std::string::npos) {
        // Get whole argument substring
        std::string parameter = request.body.substr(start, end - start);
        int equalPos = parameter.find('=');
        if (equalPos != std::string::npos) {
            // Get substring of everything before and after '='
            std::string parameterName = parameter.substr(0, equalPos);
            std::string parameterValue = parameter.substr(equalPos + 1);
            // If matches what library user gave to postForm (clientString)
            // return the parameter value
            if (parameterName == clientString)
                return parameterValue;
        }
        start = end + 1;
    }

    // Redo that for the last argument that was skipped before
    // Get whole argument substring
    std::string lastParameter = request.body.substr(start);
    int equalPos = lastParameter.find('=');
    if (equalPos != std::string::npos) {
        // Get substring of everything before and after '='
        std::string parameterName = lastParameter.substr(0, equalPos);
        std::string parameterValue = lastParameter.substr(equalPos + 1);
        // If matches what library user gave to postForm (clientString) return
        // the parameter value
        if (parameterName == clientString)
            return parameterValue;
    }

    // No matches return default given at the top
    return defaultString;
}

// Receive client data (POST etc.)
std::string HttpConnection::postForm(std::string clientString) {
    return defaultPostForm(clientString, "");
}

std::string HttpConnection::query(std::string clientString) {
    // Loop through every argument by looking for '&' (except the last argument
    // because there is no '&')
    int start = 0;
    int end;
    while ((end = request.rawQuery.find('&', start)) != std::string::npos) {
        // Get whole argument substring
        std::string parameter = request.rawQuery.substr(start, end - start);
        int equalPos = parameter.find('=');
        if (equalPos != std::string::npos) {
            // Get substring of everything before and after '='
            std::string parameterName = parameter.substr(0, equalPos);
            std::string parameterValue = parameter.substr(equalPos + 1);
            // If matches what library user gave to postForm (clientString)
            // return the parameter value
            if (parameterName == clientString)
                return parameterValue;
        }
        start = end + 1;
    }

    // Redo that for the last argument that was skipped before
    // Get whole argument substring
    std::string lastParameter = request.rawQuery.substr(start);
    int equalPos = lastParameter.find('=');
    if (equalPos != std::string::npos) {
        // Get substring of everything before and after '='
        std::string parameterName = lastParameter.substr(0, equalPos);
        std::string parameterValue = lastParameter.substr(equalPos + 1);
        // If matches what library user gave to postForm (clientString) return
        // the parameter value
        if (parameterName == clientString)
            return parameterValue;
    }

    // No matches return nothing
    return "";
}

std::string HttpConnection::param(std::string clientParam) {
    return request.param(clientParam);
}

std::string HttpConnection::getHeader(std::string clientHeader) {
    return request.header(clientHeader);
}

void HttpConnection::redirect(std::string endpoint) {
    status(302); // Not found
    header("Location", endpoint);
    close(client);
}

void HttpConnection::redirect(vesper::HttpResponse::StatusCodes statuscode,
                              std::string endpoint) {
    status(statuscode);
    header("Location", endpoint);
    close(client);
}

void HttpConnection::redirect(int statuscode, std::string endpoint) {
    status(static_cast<int>(statuscode));
    header("Location", endpoint);
    close(client);
}

void HttpConnection::setMethod(std::string method) { response.method = method; }

void HttpConnection::setClientBuffer(std::string clientBuffer) {
    request.body = clientBuffer;
}
} // namespace vesper