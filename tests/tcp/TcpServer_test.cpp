#include <gtest/gtest.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vesper/vesper.h>

TEST(TcpServer, CreateObject) {
    vesper::TcpServer server;
    SUCCEED();
}

TEST(TcpServer, ValidIPAddress) {
    vesper::TcpServer server;

    EXPECT_TRUE(server.isValidIP("127.0.0.1"));
    EXPECT_TRUE(server.isValidIP("0.0.0.0"));
    EXPECT_TRUE(server.isValidIP("192.168.1.1"));
}

TEST(TcpServer, InvalidIPAddress) {
    vesper::TcpServer server;

    EXPECT_FALSE(server.isValidIP("999.999.999.999"));
    EXPECT_FALSE(server.isValidIP("abc.def.ghi.jkl"));
    EXPECT_FALSE(server.isValidIP("127.0.0"));
    EXPECT_FALSE(server.isValidIP(""));
}

TEST(TcpServer, StartServerValid) {
    vesper::TcpServer server;

    int result = server.startServer("127.0.0.1", 0);
    EXPECT_EQ(result, 0);

    server.closeServer();
}

TEST(TcpServer, StartServerInvalidIP) {
    vesper::TcpServer server;

    int result = server.startServer("invalid_ip", 8080);
    EXPECT_NE(result, 0);
}

TEST(TcpServer, SetSocketNonBlocking) {
    vesper::TcpServer server;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(sock, 0);

    EXPECT_TRUE(server.setSocketNonBlocking(sock));

    close(sock);
}

TEST(TcpServer, OnClientSendsDefaultMessage) {
    vesper::TcpServer server;

    int sockets[2];
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    int clientSock = sockets[0];
    int testSock = sockets[1];

    auto task = server.onClient(clientSock);

    // Read what the server sent
    char buffer[128] = {0};
    ssize_t bytes = read(testSock, buffer, sizeof(buffer));

    ASSERT_GT(bytes, 0);
    EXPECT_STREQ(buffer, "No Handler parsed\n");

    close(testSock);
}

TEST(TcpServer, CloseServerWithoutStart) {
    vesper::TcpServer server;
    server.closeServer();
    SUCCEED(); // Should safely handle closing uninitialized socket
}