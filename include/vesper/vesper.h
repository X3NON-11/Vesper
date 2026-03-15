#pragma once

// #include "../http/radixTree.h"
#include "http/HttpConnection.h"
#include "http/HttpServer.h"
// #include "../tcp/TcpServer.h"
// #include "../utils/logging.h"
#include "utils/urlEncoding.h"

// So you can write Status::OK instead of this
using Status = vesper::HttpResponse::StatusCodes;