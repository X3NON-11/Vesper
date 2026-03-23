#include "../include/vesper/http/radixTree.h"

// ==========
// Radix Tree
// ==========

Tree::Tree() {
    root = std::make_unique<Node>();
    root->segment = "/";
}

void Tree::addURL(std::string url, std::string method, bool prefix,
                  std::function<void(vesper::HttpConnection &)> handler) {
    if (url == "/") {
        root->handlers[method].push_back(handler);
        root->prefix = prefix;
        return;
    }

    if (!url.empty() && url[0] == '/') {
        url.erase(0, 1); // remove leading slash
    }

    Node *current = root.get();
    int start = 0;

    while (start < url.size()) {
        int slash = url.find('/', start);
        std::string segment = (slash == std::string::npos)
                                  ? url.substr(start)
                                  : url.substr(start, slash - start);

        auto it = current->children.find(segment);
        if (it == current->children.end()) {
            // Create a new node if it doesn't exist
            auto newNode = std::make_unique<Node>();
            if (segment[0] != ':') {
                // Default node
                newNode->segment = segment;
                auto [insertedIt, inserted] =
                    current->children.emplace(segment, std::move(newNode));
                it = insertedIt;
                current = it->second.get();
            } else {
                // URL param node
                newNode->paramName = segment.substr(1); // remove ':'
                current->paramChild = std::move(newNode);
                current = current->paramChild.get();
            }
        } else {
            current = it->second.get();
        }

        if (slash == std::string::npos) {
            // Last segment: set the handler
            current->handlers[method].push_back(handler);
            current->prefix = prefix;
            return;
        }
        start = slash + 1;
    }
}

std::vector<std::function<void(vesper::HttpConnection &)>>
Tree::getNodeHandler(std::string url, std::string method) {

    if (!root)
        return {};

    if (url == "/") {
        auto it = root->handlers.find(method);
        if (it != root->handlers.end())
            return it->second;
        return {};
    }

    if (!url.empty() && url[0] == '/') {
        url.erase(0, 1);
    }

    Node *node = matchNode(url, root.get(), 0);
    if (!node)
        return {};

    auto it = node->handlers.find(method);
    if (it != node->handlers.end())
        return it->second;

    return {};
}

bool Tree::matchURL(std::string url, std::string method) {
    if (url == "/") {
        return root->handlers.find(method) != root->handlers.end();
    }

    if (!url.empty() && url[0] == '/') {
        url.erase(0, 1);
    }

    Node *node = matchNode(url, root.get(), 0);
    if (!node) {
        return false;
    }

    return node->handlers.find(method) != node->handlers.end();
}

// Only used for middleware
bool Tree::matchPrefixURL(std::string url, std::string method) {
    if (!url.empty() && url[0] == '/') {
        url.erase(0, 1);
    }

    Node *current = root.get();

    // Root-level middleware
    if (current->handlers.count(method) || current->handlers.count("ALL")) {
        return true;
    }

    int start = 0;
    while (start <= url.size()) {
        int slash = url.find('/', start);
        std::string segment = (slash == std::string::npos)
                                  ? url.substr(start)
                                  : url.substr(start, slash - start);

        if (segment.empty()) {
            break;
        }

        // Exact match first
        auto it = current->children.find(segment);
        if (it != current->children.end()) {
            if (current->prefix == true)
                current = it->second.get();
        }
        // Param match
        else if (current->paramChild) {
            if (current->prefix)
                current = current->paramChild.get();
        } else {
            return false;
        }

        if (current->handlers.count(method)) {
            if (current->prefix)
                return true;
        }

        if (slash == std::string::npos) {
            break;
        }
        start = slash + 1;
    }

    return false;
}

void Tree::collectPrefixHandlers(
    std::string url, std::string method,
    std::vector<std::function<void(vesper::HttpConnection &)>> &out) {

    if (!url.empty() && url[0] == '/') {
        url.erase(0, 1);
    }

    Node *current = root.get();

    // Root middleware
    if (current->prefix || url.empty()) {
        auto it = current->handlers.find(method);
        if (it != current->handlers.end()) {
            out.insert(out.end(), it->second.begin(), it->second.end());
        } else {
            auto all = current->handlers.find("ALL");
            if (all != current->handlers.end()) {
                out.insert(out.end(), all->second.begin(), all->second.end());
            }
        }
    }

    int start = 0;
    while (start <= url.size()) {
        int slash = url.find('/', start);
        std::string segment = (slash == std::string::npos)
                                  ? url.substr(start)
                                  : url.substr(start, slash - start);

        if (segment.empty())
            break;

        auto it = current->children.find(segment);
        if (it != current->children.end()) {
            current = it->second.get();
        } else if (current->paramChild) {
            current = current->paramChild.get();
        } else {
            break;
        }

        // Root middleware (only if prefix == true)
        if (current->prefix) {
            auto it2 = current->handlers.find(method);
            if (it2 != current->handlers.end()) {
                out.insert(out.end(), it2->second.begin(), it2->second.end());
            } else {
                auto all = current->handlers.find("ALL");
                if (all != current->handlers.end()) {
                    out.insert(out.end(), all->second.begin(),
                               all->second.end());
                }
            }
        }

        if (slash == std::string::npos)
            break;

        start = slash + 1;
    }
}

Tree::Node *Tree::matchNode(std::string &url, Tree::Node *currentNode,
                            int startSlash) {
    int slash = url.find('/', startSlash);

    std::string segment = (slash == std::string::npos)
                              ? url.substr(startSlash)
                              : url.substr(startSlash, slash - startSlash);

    Tree::Node *nextNode = nullptr;

    // Exact match first
    auto it = currentNode->children.find(segment);
    if (it != currentNode->children.end()) {
        nextNode = it->second.get();
    }
    // Param match
    else if (currentNode->paramChild) {
        nextNode = currentNode->paramChild.get();
    } else {
        return nullptr;
    }

    if (slash == std::string::npos) {
        return nextNode;
    }

    return matchNode(url, nextNode, slash + 1);
}

std::unordered_map<std::string, std::string>
Tree::getUrlParams(std::string url, std::string method) {
    std::unordered_map<std::string, std::string> params;

    if (!url.empty() && url[0] == '/') {
        url.erase(0, 1);
    }

    Node *currentNode = root.get();
    int start = 0;

    while (start <= url.size()) {
        int slash = url.find('/', start);
        std::string segment = (slash == std::string::npos)
                                  ? url.substr(start)
                                  : url.substr(start, slash - start);

        if (segment.empty())
            break;

        auto it = currentNode->children.find(segment);
        if (it != currentNode->children.end()) {
            currentNode = it->second.get();
        } else if (currentNode->paramChild) {
            currentNode = currentNode->paramChild.get();
            params[currentNode->paramName] = segment;
        } else {
            return {}; // no match
        }

        if (slash == std::string::npos)
            break;
        start = slash + 1;
    }

    if (currentNode->handlers.find(method) == currentNode->handlers.end()) {
        return {};
    }

    return params;
}