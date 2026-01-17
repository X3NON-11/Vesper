#include "include/server.h"

int main() {
    http::TcpServer server = http::TcpServer("localhost", 8080);
    return 0;
}