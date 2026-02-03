#pragma once

#include "include/http/radixTree.h"
#include "include/http/HttpConnection.h"
#include "include/http/HttpServer.h"
#include "include/tcp/TcpServer.h"
#include "include/utils/logging.h"
#include "include/utils/urlEncoding.h"

// So you can write Status::OK instead of this
using Status = vesper::HttpResponse::StatusCodes;