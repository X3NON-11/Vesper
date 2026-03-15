#pragma once

#include <string>           // std::string::npos
#include <functional>       // std::function
#include <coroutine>

#include "../utils/logging.h"
#include "../http/radixTree.h"     // Used for the tries that saves all the endpoints and middlewares
#include "../utils/configParser.h" // The abstractions for glaze

namespace vesper {
    class HttpServer;
}

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
            std::string method;
            std::unordered_map<std::string, std::string> headers;

            HttpResponse(StatusCodes status, std::string body, std::string type, std::string method);
            std::string toHttpString();
            void setHeader(const std::string& name, const std::string& value);
            void removeHeader(std::string& name);
        private:
            std::string statusToString(StatusCodes status);
    };
    
    // Used to store all details about the client request
    class HttpRequest {
        public:
            // Core HTTP
            std::string method;
            std::string path;
            std::string httpVersion;
        
            // Query & params
            std::string rawQuery;
            std::unordered_map<std::string, std::string> query;
            std::unordered_map<std::string, std::string> params;
        
            // Headers & body
            std::unordered_map<std::string, std::string> headers;
            std::string body;
        
            // Abstractions for convenience
            std::string header(std::string name) {
                auto it = headers.find(std::string(name));
                return it != headers.end() ? it->second : "";
            }
        
            std::string param(std::string name) {
                auto it = params.find(std::string(name));
                return it != params.end() ? it->second : "";
            }
        
            std::string queryParam(std::string name) {
                auto it = query.find(std::string(name));
                return it != query.end() ? it->second : "";
            }
        };

    // Stores all the http abstractions / what you would access as a library user (e.g c.string("Hello World"); )
    class HttpConnection {
        private:
            vesper::HttpServer* server;
            int client;
            std::function<void()> nextFn;

        public:
            std::string domain = "";
            // Stores all information about the request (from the client)
            HttpRequest request;
            // How the user can access all response internal variables/functions
            HttpResponse response{HttpResponse::StatusCodes::OK, "", "text/plain", "GET"};

            explicit HttpConnection(int client, vesper::HttpServer *server);

            void setMethod(std::string method);
            void setClientBuffer(std::string bodyBuffer);
            void sendErrorNoHandler();
            void redirect(std::string endpoint);
            void redirect(vesper::HttpResponse::StatusCodes statuscode, std::string endpoint);
            void redirect(int statuscode, std::string endpoint);

            // Abstractions for convenience (only calls data())
            void string(int status, std::string body);
            void string(HttpResponse::StatusCodes status, std::string body);
            void string(std::string body); // Default status: 200
            void json(int status, std::string jsonBody);
            void json(HttpResponse::StatusCodes status, std::string jsonBody);
            void json(std::string jsonBody); // Default status: 200
            void status(int status);
            void status(HttpResponse::StatusCodes status);
            void header(std::string hName, std::string hContent);

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
            void setCookie(std::string name, std::string value, int maxAge, std::string path, std::string domain, bool secure, bool httpOnly);
            void setCookie(std::string name, std::string value, int maxAge, bool secure, bool httpOnly);
            void setCookie(std::string name, std::string value, bool secure, bool httpOnly);
            void setCookie(std::string name, std::string value);
            std::string cookies(std::string clientString);
            
            // Glaze abstractions called in configParser.h
            template <typename T> [[nodiscard]]
            std::string shouldBindJson(const T& value) { return bindJson(value); }
            template <typename T> [[nodiscard]]
            std::string shouldBindYaml(const T& value) { return bindYaml(value); }
            template <typename T> [[nodiscard]]
            std::string shouldBindToml(const T& value) { return bindToml(value); }
    };
}