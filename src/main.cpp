#include "include/server.h"

// ====================
// All functions
// ====================
void myHandler(http::HttpConnection& conn);
void testEndpoint(http::HttpConnection& conn);

int main() {
    // Start the server
    http::HttpServer server("localhost", 8080);
    server.setHandler(myHandler); // Fallback non existent endpoint & used for endpoint /
    server.GET("/test", testEndpoint);
    server.run();
}

void myHandler(http::HttpConnection& conn) {
    conn.sendPlainText("Hello World\n"); // Default status: 200
}

void testEndpoint(http::HttpConnection& conn) {
    conn.sendPlainText("Test\n");
}