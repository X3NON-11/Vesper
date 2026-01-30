#pragma once

#include "logging.h"

#include <unordered_map>
#include <functional>
#include <memory>
#include <string>

namespace http {
    class HttpConnection;
}

class Tree {
    public:
        Tree();
        void addURL(std::string url, std::string method, std::function<void(http::HttpConnection&)> handler);
        std::function<void(http::HttpConnection&)> getNodeHandler(std::string url, std::string method);
        bool matchURL(std::string url, std::string method);

        struct Node {
            std::unordered_map<std::string, std::unique_ptr<Node>> children;

            std::string segment;
            std::unordered_map<std::string, std::function<void(http::HttpConnection&)>> handlers;
            Node* paramChild = nullptr;
            std::string paramName;
        };
        std::unique_ptr<Node> root;
};