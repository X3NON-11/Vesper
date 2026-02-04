#include <vesper.h>

// ==============================
//         All functions
// ==============================
void myHandler(vesper::HttpConnection &c);
void testEndpoint(vesper::HttpConnection &c);
void testMiddleware(vesper::HttpConnection &c);
void postEndpoint(vesper::HttpConnection &c);
void queryHandler(vesper::HttpConnection &c);
void userIdHandler(vesper::HttpConnection &c);
void headerHandler(vesper::HttpConnection &c);

int main() {
    debugging = true;       // Default on
    timeDebugging = false;  // Default on
    ignoreWarnings = false; // Default off

    // Start the server
    vesper::HttpServer server;

    // Route handlers
    // server.setMiddleware("/test", "ALL", testMiddleware);
    server.GET("/", myHandler);                        // Website endpoint
    server.GET("/test", testMiddleware, testEndpoint); // JSON endpoint
    server.POST("/post", postEndpoint);
    server.GET("/query", queryHandler);
    server.GET("/user/:id/test", testMiddleware, userIdHandler);
    server.GET("/header", headerHandler);

    server.run("localhost", 8080);
}

// Default handler: serve a small HTML page
void myHandler(vesper::HttpConnection &c) {
    const char *html = R"(
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
void testEndpoint(vesper::HttpConnection &c) {
    const char *json = R"(
        {
            "status": "OK",
            "message": "This is a test JSON response"
        }
    )";

    c.json(json);
    c.redirect("/");
}

void testMiddleware(vesper::HttpConnection &c) {
    c.string("Middleware Started\n\n");
    c.next();
    c.string("\n\nMiddleware Ended\n");
}

void postEndpoint(vesper::HttpConnection &c) {
    std::string message = c.postForm("test");
    message.empty() ? c.string("No message") : c.string(message);
}

void queryHandler(vesper::HttpConnection &c) {
    std::string message = c.query("test");
    !message.empty() ? c.string(message) : c.string("No message\n");
}

void userIdHandler(vesper::HttpConnection &c) {
    std::string clientID = c.param("id");
    clientID != "" ? c.string(clientID) : c.string("No ID parsed");
}

void headerHandler(vesper::HttpConnection &c) {
    std::string message = c.getHeader("test");
    c.string(message);
}