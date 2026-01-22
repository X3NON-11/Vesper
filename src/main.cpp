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
    conn.sendPlainText("Second message");
}

void testEndpoint(http::HttpConnection& conn) {
//    conn.sendPlainText("Test1\n");
//    conn.sendPlainText("Test2\n");

    conn.data("application/json", http::HttpResponse::StatusCodes::OK, "{\n Test,\n Message \n}");
}