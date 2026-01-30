#include "include/radixTree.h"

void Tree::addURL(std::string url, std::string method, std::function<void(http::HttpConnection&)> handler) {
    if (url == "/") {
        root->handlers[method] = handler;
        return;
    }

    if (!url.empty() && url[0] == '/') {
        url.erase(0, 1); // remove leading slash
    }

    Node* current = root.get();
    int start = 0;

    while (start < url.size()) {
        int slash = url.find('/', start);
        std::string segment = (slash == std::string::npos) ? url.substr(start) : url.substr(start, slash - start);

        auto it = current->children.find(segment);
        if (it == current->children.end()) {
            // Create a new node if it doesn't exist
            auto newNode = std::make_unique<Node>();
            //if (segment[0] != ':') {
                newNode->segment = segment;
                current->children[segment] = std::move(newNode);
                it = current->children.find(segment);
            //}
        }

        current = it->second.get();

        if (slash == std::string::npos) {
            // Last segment: set the handler
            current->handlers[method] = handler;
            return;
        }

        start = slash + 1;
    }
}

std::function<void(http::HttpConnection&)> Tree::getNodeHandler(std::string url, std::string method) {
    if (!root) return nullptr;

    if (url == "/") {
        auto h = root->handlers.find(method);
        if (h == root->handlers.end()) {
            return nullptr;
        }
        return h->second;
    }

    if (!url.empty() && url[0] == '/') {
        url.erase(0, 1);
    }
    Node* current = root.get();
    int start = 0;

    while (true) {
        int slash = url.find('/', start);
        std::string segment = (slash == std::string::npos) ? url.substr(start) : url.substr(start, slash - start);
        auto it = current->children.find(segment);
        if (it == current->children.end()) {
            return nullptr; // no such route
        }
        current = it->second.get();
        if (slash == std::string::npos) {
            break; // end of path
        }
        start = slash + 1;
    }

    auto h = current->handlers.find(method);
    if (h == current->handlers.end()) {
        return nullptr;
    }
    return h->second;
}

bool Tree::matchURL(std::string url, std::string method) {
    // root endpoint special case
    if (url == "/") {
        auto h = root->handlers.find(method);
        return h != root->handlers.end();
    }

    if (!url.empty() && url[0] == '/') {
        url.erase(0, 1);
    }
    Node* current = root.get();
    int start = 0;
    while (true) {
        int slash = url.find('/', start);
        std::string segment = (slash == std::string::npos) ? url.substr(start) : url.substr(start, slash - start);
        auto it = current->children.find(segment);
        if (it == current->children.end()) {
            return false; // no such route
        }
        current = it->second.get();
        if (slash == std::string::npos) {
            auto h = current->handlers.find(method);
            return h != current->handlers.end();
        }
        start = slash + 1;
    }
}

Tree::Tree() {
    root = std::make_unique<Node>();
    root->segment = "/";
}
