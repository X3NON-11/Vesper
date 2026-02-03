#pragma once

#include <string>           // std::string::npos
#include <functional>       // std::function
#include <sys/socket.h>     // socket, bind, listen

#include "../utils/logging.h"

namespace vesper {
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
            std::string clientBuffer; // Used for POST (etc.) requests
            std::string bodyBuffer;
            std::string type;
            std::string method;
            std::function<void()> nextFn;

        public:
            std::string clientHeader;
            std::string clientEndpoint;
            std::string clientMethod;
            std::string clientHttpVersion;
            std::string clientQuery;
            std::unordered_map<std::string, std::string> clientParams; // 1.String: paramName 2.String: content
            std::unordered_map<std::string, std::string> clientHeaders; // 1.String: headerName 2.String: content

            explicit HttpConnection(int client);
            void setMethod(std::string method);
            void setClientBuffer(std::string bodyBuffer);
            void sendErrorNoHandler();
            void sendBuffer(); // Sends all cashed responses together
            void sendBuffer(std::string type, HttpResponse::StatusCodes status);

            // Abstractions for convenience (only calls data())
            void string(int status, std::string body);
            void string(HttpResponse::StatusCodes status, std::string body);
            void string(std::string body); // Default status: 200
            void json(int status, std::string jsonBody);
            void json(HttpResponse::StatusCodes status, std::string jsonBody);
            void json(std::string jsonBody); // Default status: 200
            void status(int status);
            void status(HttpResponse::StatusCodes status);

            // Handles/Sends every supported data type by storing it correctly in the bodyBuffer
            void data(std::string type, int status, std::string body);
            void data(std::string type, HttpResponse::StatusCodes status, std::string body);
            void data(std::string type, std::string body);

            // Used in middleware to stop at this point, execute next middleware, then proceed
            void next();
            // Used to set the next middleware for the HttpConnection from HttpServer
            void setNext(std::function<void()> fn);

            // Receive client data (POST etc.)
            std::string postForm(std::string clientString);
            std::string defaultPostForm(std::string clientString, std::string defaultString);
            std::string query(std::string clientString);
            std::string param(std::string clientParam);
            std::string getHeader(std::string clientHeader);
    };
}