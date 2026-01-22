#include "http-server.h"

// ====================
// All functions
// ====================
void myHandler(http::HttpConnection& conn);
void testEndpoint(http::HttpConnection& conn);

int main() {
    // Start the server
    http::HttpServer server("localhost", 8080);

    // Route handlers
    server.setHandler(myHandler);       // Default handler for /
    server.GET("/test", testEndpoint);  // JSON endpoint

    server.run();
}

// Default handler: serve a small HTML page
void myHandler(http::HttpConnection& conn) {
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

    conn.data("text/html", Status::OK, html); // Using conn.data as you wrote
}

// Test endpoint: return JSON
void testEndpoint(http::HttpConnection& conn) {
    const char* json = R"(
        {
            "status": "OK",
            "message": "This is a test JSON response"
        }
    )";

    conn.data("application/json", Status::OK, json); // Using conn.data as in your example
}