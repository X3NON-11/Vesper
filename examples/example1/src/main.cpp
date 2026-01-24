#include "http-server.h"

// ==============================
//         All functions
// ==============================
void myHandler(http::HttpConnection& c);
void testEndpoint(http::HttpConnection& c);

int main() {
    debugging = true; // Default on
    timeDebugging = false; // Default on

    // Start the server
    http::HttpServer server;

    // Route handlers
    server.GET("/", myHandler);         // Website endpoint
    server.GET("/test", testEndpoint);  // JSON endpoint

    server.run("localhost", 8080);
}

// Default handler: serve a small HTML page
void myHandler(http::HttpConnection& c) {
    const char* html = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <title>My Small Website</title>
            <style>
                body { font-family: Arial, sans-serif; background: #f0f0f0; padding: 20px; }
                h1 { color: #333; }
                p { color: #666; }
                a { color: #0066cc; }
            </style>
        </head>
        <body>
            <h1>Welcome to the Http-Server Example</h1>
            <p>This is a simple C++ HTTP server example.</p>
            <p>Try visiting the <a href="/test">/test</a> endpoint for JSON output.</p>
        </body>
        </html>
    )";

    c.data("text/html", Status::OK, html);
}

// Test endpoint: return JSON
void testEndpoint(http::HttpConnection& c) {
    const char* json = R"(
        {
            "status": "OK",
            "message": "This is a test JSON response"
        }
    )";

    c.json(json);
}
