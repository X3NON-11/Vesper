#include <gtest/gtest.h>
#include <vesper/vesper.h>

using namespace vesper;

// ============
// HttpResponse
// ============

TEST(HttpResponse, ConstructorStoresValues) {
    HttpResponse res(HttpResponse::StatusCodes::OK, "hello", "text/plain",
                     "GET");

    EXPECT_EQ(res.body, "hello");
    EXPECT_EQ(res.contentType, "Content-Type: text/plain; charset=utf-8");
    EXPECT_EQ(res.method, "GET");
}

TEST(HttpResponse, SetHeader) {
    HttpResponse res(HttpResponse::StatusCodes::OK, "", "text/plain", "GET");

    res.setHeader("Content-Type", "application/json");

    EXPECT_EQ(res.headers["Content-Type"], "application/json");
}

TEST(HttpResponse, RemoveHeader) {
    HttpResponse res(HttpResponse::StatusCodes::OK, "", "text/plain", "GET");

    res.setHeader("X-Test", "123");
    std::string key = "X-Test";
    res.removeHeader(key);

    EXPECT_TRUE(res.headers.find("X-Test") == res.headers.end());
}

TEST(HttpResponse, ToHttpStringContainsBody) {
    HttpResponse res(HttpResponse::StatusCodes::OK, "Hello", "text/plain",
                     "GET");

    std::string http = res.toHttpString();

    EXPECT_NE(http.find("Hello"), std::string::npos);
}

TEST(HttpResponse, ToHttpStringContainsStatus) {
    HttpResponse res(HttpResponse::StatusCodes::NOT_FOUND, "missing",
                     "text/plain", "GET");

    std::string http = res.toHttpString();

    EXPECT_NE(http.find("404"), std::string::npos);
}

// ===========
// HttpRequest
// ===========

TEST(HttpRequest, HeaderLookup) {
    HttpRequest req;
    req.headers["Content-Type"] = "application/json";

    EXPECT_EQ(req.header("Content-Type"), "application/json");
}

TEST(HttpRequest, HeaderMissingReturnsEmpty) {
    HttpRequest req;

    EXPECT_EQ(req.header("Missing"), "");
}

TEST(HttpRequest, QueryLookup) {
    HttpRequest req;
    req.query["page"] = "10";

    EXPECT_EQ(req.queryParam("page"), "10");
}

TEST(HttpRequest, ParamLookup) {
    HttpRequest req;
    req.params["id"] = "42";

    EXPECT_EQ(req.param("id"), "42");
}

// ==============
// HttpConnection
// ==============

TEST(HttpConnection, ConstructConnection) {
    HttpConnection conn(-1, nullptr);

    SUCCEED();
}

TEST(HttpConnection, SetMethod) {
    HttpConnection conn(-1, nullptr);

    conn.setMethod("POST");

    EXPECT_EQ(conn.response.method, "POST");
}

TEST(HttpConnection, StringHelperSetsBody) {
    HttpConnection conn(-1, nullptr);

    conn.string("hello world");

    EXPECT_EQ(conn.response.body, "hello world");
}

TEST(HttpConnection, JsonHelperSetsContentType) {
    HttpConnection conn(-1, nullptr);

    conn.json("{\"a\":1}");

    EXPECT_EQ(conn.response.contentType, "application/json");
}

TEST(HttpConnection, StatusSetter) {
    HttpConnection conn(-1, nullptr);

    conn.status(404);

    EXPECT_EQ((int)conn.response.status, 404);
}

TEST(HttpConnection, HeaderSetter) {
    HttpConnection conn(-1, nullptr);

    conn.header("X-Test", "abc");

    EXPECT_EQ(conn.response.headers["X-Test"], "abc");
}

TEST(HttpConnection, CookieSetterBasic) {
    HttpConnection conn(-1, nullptr);

    conn.setCookie("session", "123");

    EXPECT_TRUE(conn.response.headers.find("Set-Cookie") !=
                conn.response.headers.end());
}

TEST(HttpConnection, QueryHelper) {
    HttpConnection conn(-1, nullptr);
    conn.request.rawQuery = "page=3";

    EXPECT_EQ(conn.query("page"), "3");
}

TEST(HttpConnection, ParamHelper) {
    HttpConnection conn(-1, nullptr);
    conn.request.params["id"] = "99";

    EXPECT_EQ(conn.param("id"), "99");
}

TEST(HttpConnection, HeaderHelper) {
    HttpConnection conn(-1, nullptr);
    conn.request.headers["User-Agent"] = "Test";

    EXPECT_EQ(conn.getHeader("User-Agent"), "Test");
}