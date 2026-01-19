#include "include/server.h"

int main() {
    // Start the server
    http::HttpServer server("localhost", 8080);
    server.run();

    return 0;
}