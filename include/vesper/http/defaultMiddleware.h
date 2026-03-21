#pragma once

#include "../utils/logging.h"
#include "HttpConnection.h"

inline void recoveryMiddleware(vesper::HttpConnection &c) {
    try {
        c.next();
    } catch (const std::exception& err) {
        log(LogType::Error, err.what());
    }
}

inline void loggingMiddleware(vesper::HttpConnection &c) {
    c.next();
    logConnection(static_cast<int>(c.response.status),
                  c.response.method, c.request.path);
}