#include <gtest/gtest.h>
#include <vesper/vesper.h>

TEST(CreateTreeObject, ExpectSuccess) {
    Tree t;
    SUCCEED();
}

TEST(AddURL, ExpectSuccess) {
    Tree t;
    t.addURL("/test/case", "GET", false,
             [](vesper::HttpConnection &) { return; });
    SUCCEED();
}

TEST(getNodeHandler, ExpectSuccess) {
    Tree t;
    t.addURL("/test/case", "GET", false,
             [](vesper::HttpConnection &) { return; });
    auto node = t.getNodeHandler("/test/case", "GET");
    EXPECT_TRUE(node.back());
}

TEST(matchURL, ExpectSuccess) {
    Tree t;
    t.addURL("/test/case", "GET", false,
             [](vesper::HttpConnection &) { return; });

    EXPECT_TRUE(t.matchURL("/test/case", "GET"));
}

TEST(matchURL, ExpectFail) {
    Tree t;

    EXPECT_FALSE(t.matchURL("/non/existing/url", "GET"));
}

TEST(matchiPrefixURL, ExpectFail) {
    Tree t;
    t.addURL("/test/case", "GET", true,
             [](vesper::HttpConnection &) { return; });

    EXPECT_FALSE(t.matchPrefixURL("/non/existing/prefix", "GET"));
}

TEST(singleParam, ExpectEqual) {
    Tree t;
    t.addURL("/users/:id", "GET", false,
             [](vesper::HttpConnection &) { return; });

    auto params = t.getUrlParams("/users/42", "GET");

    EXPECT_EQ(params["id"], "42");
}

TEST(GetUrlParams, multipleParams) {
    Tree t;
    t.addURL("/users/:userId/posts/:postId", "GET", false,
             [](vesper::HttpConnection &) { return; });

    auto params = t.getUrlParams("/users/10/posts/55", "GET");

    EXPECT_EQ(params["userId"], "10");
    EXPECT_EQ(params["postId"], "55");
}

TEST(CollectPrefixHandlers, singlePrefix) {
    Tree t;
    t.addURL("/api", "GET", true, [](vesper::HttpConnection &) { return; });

    std::vector<std::function<void(vesper::HttpConnection &)>> handlers;
    t.collectPrefixHandlers("/api/users", "GET", handlers);

    EXPECT_EQ(handlers.size(), 1);
}

TEST(CollectPrefixHandlers, multiplePrefixes) {
    Tree t;
    t.addURL("/api", "GET", true, [](vesper::HttpConnection &) { return; });
    t.addURL("/api/users", "GET", true,
             [](vesper::HttpConnection &) { return; });

    std::vector<std::function<void(vesper::HttpConnection &)>> handlers;
    t.collectPrefixHandlers("/api/users/123", "GET", handlers);

    EXPECT_EQ(handlers.size(), 2);
}