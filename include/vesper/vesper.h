#pragma once

// #include "../http/radixTree.h"
#include "http/HttpConnection.h"
#include "http/HttpServer.h"
// #include "../tcp/TcpServer.h"
// #include "../utils/logging.h"
#include "utils/urlEncoding.h"

// So you can write Status::OK instead of this
namespace vesper {
    using enum HttpResponse::StatusCodes;
    using enum  vesper::HttpServerTypes;
}