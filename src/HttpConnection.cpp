#include "include/server.h"

namespace http {    // HTTP-RESPONSE used to convert http logic to tcp logic
    HttpResponse::HttpResponse(StatusCodes status, std::string body, std::string type) : status(status), body(body) {
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
        return "HTTP/1.1 " +
            std::to_string(static_cast<int>(status)) + " " +
            statusToString(status) + "\r\n" +
            "Content-Type: " + contentType + "\r\n" +
            "Content-Length: " + std::to_string(body.size()) + "\r\n" +
            "\r\n" +
            body;
    }
    
    std::string HttpResponse::statusToString(StatusCodes status) {
        switch (status) {
            case StatusCodes::OK: return "OK";
            case StatusCodes::BAD_REQUEST: return "Bad Request";
            case StatusCodes::UNAUTHORIZED: return "Unauthorized";
            case StatusCodes::FORBIDDEN: return "Forbidden";
            case StatusCodes::NOT_FOUND: return "Not Found";
            case StatusCodes::TOO_MANY_REQUESTS: return "Too Many Requests";
            case StatusCodes::INTERNAL_SERVER_ERROR: return "Internal Server Error";
            default: return "Unknown";
        }
    }
}

namespace http {    // HTTP-CONNECTION responsible for translating abstractions to tcp usable format
    HttpConnection::HttpConnection(int client) : client(client) {}

    // All abstractions like c.string to send plain text
    void HttpConnection::sendErrorNoHandler() {
        string(HttpResponse::StatusCodes::INTERNAL_SERVER_ERROR, "No handler parsed");
        log(LogType::Warn, "No handler parsed");
    }
    
    void HttpConnection::string(int status, std::string body) {
        if (bodyBuffer != "") log(LogType::Warn, "Only one message is intended - the first content header applies to both");
        bodyBuffer += body;
        type = "text/plain";
        responseStatus = static_cast<HttpResponse::StatusCodes>(status);
    }
    void HttpConnection::string(HttpResponse::StatusCodes status, std::string body) {
        if (bodyBuffer != "") log(LogType::Warn, "Only one message is intended - the first content header applies to both");
        bodyBuffer += body;
        type = "text/plain";
        responseStatus = status;
    }
    void HttpConnection::string(std::string body) {
        string(HttpResponse::StatusCodes::OK, body);
    }

    void HttpConnection::json(int status, std::string jsonBody) {
        if (bodyBuffer != "") log(LogType::Warn, "Only one message is intended - the first content header applies to both");
        bodyBuffer += jsonBody;
        type = "application/json";
        responseStatus = static_cast<HttpResponse::StatusCodes>(status);
    }
    void HttpConnection::json(HttpResponse::StatusCodes status, std::string jsonBody) {
        if (bodyBuffer != "") log(LogType::Warn, "Only one message is intended - the first content header applies to both");
        bodyBuffer += jsonBody;
        type = "application/json";
        responseStatus = status;
    }
    void HttpConnection::json(std::string jsonBody) {
        json(HttpResponse::StatusCodes::OK, jsonBody);
    }

    void HttpConnection::status(int status) {
        responseStatus = static_cast<HttpResponse::StatusCodes>(status);
    }
    void HttpConnection::status(HttpResponse::StatusCodes status) {
        responseStatus = status;
    }

    void HttpConnection::data(std::string type, int status, std::string body) {
        if (bodyBuffer != "") log(LogType::Warn, "Only one message is intended - the first content header applies to both");
        bodyBuffer += body;
        this->type = type;
        responseStatus = static_cast<HttpResponse::StatusCodes>(status);
    }
    void HttpConnection::data(std::string type, HttpResponse::StatusCodes status, std::string body) {
        if (bodyBuffer != "") log(LogType::Warn, "Only one message is intended - the first content header applies to both");
        bodyBuffer += body;
        this->type = type;
        responseStatus = status;
    }
    void HttpConnection::data(std::string type, std::string body) {
        data(type, HttpResponse::StatusCodes::OK, body);
    }

    // Buffer for messages
    void HttpConnection::sendBuffer(std::string type, HttpResponse::StatusCodes status) {
        HttpResponse response(status, bodyBuffer, type);
        std::string http = response.toHttpString();

        send(client, http.c_str(), http.size(), 0);
    }

    void HttpConnection::sendBuffer() {
        sendBuffer(type, responseStatus);
    }

    // Middleware
    // Used to set the next middleware for the HttpConnection from HttpServer
    void HttpConnection::setNext(std::function<void()> fn) { 
        nextFn = fn;
    }

    // Used in middleware to stop at this point, execute next middleware, then proceed
    void HttpConnection::next() {
        if (nextFn) {
            nextFn();
            nextFn = nullptr;
        }
    }

        // Receive client data (POST etc.)
    std::string HttpConnection::defaultPostForm(std::string clientString, std::string defaultString) {
        if (method == "GET") {
            return defaultString;
        }

        // Loop through every argument by looking for '&' (except the last argument because there is no '&')
        int start = 0;
        int end;
        while ((end = clientBuffer.find('&', start)) != std::string::npos) {
            // Get whole argument substring
            std::string parameter = clientBuffer.substr(start, end - start);
            int equalPos = parameter.find('=');
            if (equalPos != std::string::npos) {
                // Get substring of everything before and after '='
                std::string parameterName = parameter.substr(0, equalPos);
                std::string parameterValue = parameter.substr(equalPos + 1);
                // If matches what library user gave to postForm (clientString) return the parameter value
                if (parameterName == clientString) return parameterValue;
            }
            start = end + 1;
        }

        // Redo that for the last argument that was skipped before
        // Get whole argument substring
        std::string lastParameter = clientBuffer.substr(start);
        int equalPos = lastParameter.find('=');
        if (equalPos != std::string::npos) {
            // Get substring of everything before and after '='
            std::string parameterName = lastParameter.substr(0, equalPos);
            std::string parameterValue = lastParameter.substr(equalPos + 1);
            // If matches what library user gave to postForm (clientString) return the parameter value
            if (parameterName == clientString) return parameterValue;
        }

        // No matches return default given at the top
        return defaultString;
    }

    // Receive client data (POST etc.)
    std::string HttpConnection::postForm(std::string clientString) {
        return defaultPostForm(clientString, "");
    }

    std::string HttpConnection::query(std::string clientString) {
        // Loop through every argument by looking for '&' (except the last argument because there is no '&')
        int start = 0;
        int end;
        while ((end = clientQuery.find('&', start)) != std::string::npos) {
            // Get whole argument substring
            std::string parameter = clientQuery.substr(start, end - start);
            int equalPos = parameter.find('=');
            if (equalPos != std::string::npos) {
                // Get substring of everything before and after '='
                std::string parameterName = parameter.substr(0, equalPos);
                std::string parameterValue = parameter.substr(equalPos + 1);
                // If matches what library user gave to postForm (clientString) return the parameter value
                if (parameterName == clientString) return parameterValue;
            }
            start = end + 1;
        }

        // Redo that for the last argument that was skipped before
        // Get whole argument substring
        std::string lastParameter = clientQuery.substr(start);
        int equalPos = lastParameter.find('=');
        if (equalPos != std::string::npos) {
            // Get substring of everything before and after '='
            std::string parameterName = lastParameter.substr(0, equalPos);
            std::string parameterValue = lastParameter.substr(equalPos + 1);
            // If matches what library user gave to postForm (clientString) return the parameter value
            if (parameterName == clientString) return parameterValue;
        }

        // No matches return ""
        return "";
    }

    void HttpConnection::setMethod(std::string method) {
        this->method = method;
    }

    void HttpConnection::setClientBuffer(std::string clientBuffer) {
        this->clientBuffer = clientBuffer;
    }
}