#pragma once

#include "../utils/logging.h"

#include <unordered_map>
#include <functional>
#include <memory>
#include <string>

namespace vesper {
    class HttpConnection;
}

class Tree {
    public:
        struct Node {
            std::unordered_map<std::string, std::unique_ptr<Node>> children;

            std::string segment;
            std::unordered_map<std::string, std::vector<std::function<void(vesper::HttpConnection&)>>> handlers;
            std::unique_ptr<Node> paramChild = nullptr;
            std::string paramName;
            bool prefix = false;
        };
        std::unique_ptr<Node> root;
        
        Tree();
        void addURL(std::string url, std::string method, bool prefix, std::function<void(vesper::HttpConnection&)> handler);
        std::vector<std::function<void(vesper::HttpConnection &)>> getNodeHandler(std::string url, std::string method);
        bool matchURL(std::string url, std::string method);
        bool matchPrefixURL(std::string url, std::string method); // Only used for middleware
        std::unordered_map<std::string, std::string> getUrlParams(std::string url, std::string method);
        void collectPrefixHandlers(std::string url, std::string method, std::vector<std::function<void(vesper::HttpConnection &)>> &out);
      
    private:
        Node* matchNode(std::string &url, Node *currentNode, int startSlash);
};