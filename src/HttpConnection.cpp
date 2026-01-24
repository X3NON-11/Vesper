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
            contentType + "\r\n" +
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

    void HttpConnection::string(HttpResponse::StatusCodes status, std::string body) {
        bodyBuffer += body;
        type = "text/plain";
    }
    void HttpConnection::string(std::string body) {
        string(HttpResponse::StatusCodes::OK, body);
    }

    void HttpConnection::json(HttpResponse::StatusCodes status, std::string jsonBody) {
        bodyBuffer += jsonBody;
        type = "application/json";
        responseStatus = status;
    }
    void HttpConnection::json(std::string jsonBody) {
        json(HttpResponse::StatusCodes::OK, jsonBody);
    }

    void HttpConnection::status(HttpResponse::StatusCodes status) {
        responseStatus = status;
    }

    void HttpConnection::data(std::string type, HttpResponse::StatusCodes status, std::string body) {
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
}