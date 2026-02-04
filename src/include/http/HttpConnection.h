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
            // I know it needs to be overhauled but I am too lazy right now
            enum class StatusCodes : int {
                // 1xx Informational
                CONTINUE = 100,
                SWITCHING_PROTOCOLS = 101,
                PROCESSING = 102,
                EARLY_HINTS = 103,
            
                // 2xx Success
                OK = 200,
                CREATED = 201,
                ACCEPTED = 202,
                NON_AUTHORITATIVE_INFORMATION = 203,
                NO_CONTENT = 204,
                RESET_CONTENT = 205,
                PARTIAL_CONTENT = 206,
                MULTI_STATUS = 207,
                ALREADY_REPORTED = 208,
                IM_USED = 226,
            
                // 3xx Redirection
                MULTIPLE_CHOICES = 300,
                MOVED_PERMANENTLY = 301,
                FOUND = 302,
                SEE_OTHER = 303,
                NOT_MODIFIED = 304,
                USE_PROXY = 305,
                TEMPORARY_REDIRECT = 307,
                PERMANENT_REDIRECT = 308,
            
                // 4xx Client Error
                BAD_REQUEST = 400,
                UNAUTHORIZED = 401,
                PAYMENT_REQUIRED = 402,
                FORBIDDEN = 403,
                NOT_FOUND = 404,
                METHOD_NOT_ALLOWED = 405,
                NOT_ACCEPTABLE = 406,
                PROXY_AUTHENTICATION_REQUIRED = 407,
                REQUEST_TIMEOUT = 408,
                CONFLICT = 409,
                GONE = 410,
                LENGTH_REQUIRED = 411,
                PRECONDITION_FAILED = 412,
                PAYLOAD_TOO_LARGE = 413,
                URI_TOO_LONG = 414,
                UNSUPPORTED_MEDIA_TYPE = 415,
                RANGE_NOT_SATISFIABLE = 416,
                EXPECTATION_FAILED = 417,
                IM_A_TEAPOT = 418,
                MISDIRECTED_REQUEST = 421,
                UNPROCESSABLE_ENTITY = 422,
                LOCKED = 423,
                FAILED_DEPENDENCY = 424,
                TOO_EARLY = 425,
                UPGRADE_REQUIRED = 426,
                PRECONDITION_REQUIRED = 428,
                TOO_MANY_REQUESTS = 429,
                REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
                UNAVAILABLE_FOR_LEGAL_REASONS = 451,
            
                // 5xx Server Error
                INTERNAL_SERVER_ERROR = 500,
                NOT_IMPLEMENTED = 501,
                BAD_GATEWAY = 502,
                SERVICE_UNAVAILABLE = 503,
                GATEWAY_TIMEOUT = 504,
                HTTP_VERSION_NOT_SUPPORTED = 505,
                VARIANT_ALSO_NEGOTIATES = 506,
                INSUFFICIENT_STORAGE = 507,
                LOOP_DETECTED = 508,
                NOT_EXTENDED = 510,
                NETWORK_AUTHENTICATION_REQUIRED = 511
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
            void redirect(std::string endpoint);

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