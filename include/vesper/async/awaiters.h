#pragma once

#include <coroutine>
#include <sys/socket.h>
#include <unistd.h>

#include "eventLoop.h"

namespace vesper::async {

struct AcceptResult {
    int fd;
    enum class Status {
        Ok,
        Retry,
        Closed,
        Error
    } status;
};
struct AcceptAwaiter {
    int listenSocket;

    explicit AcceptAwaiter(int s)
        : listenSocket(s) {}

    bool await_ready() const noexcept {
        return false;
    }

    void await_suspend(std::coroutine_handle<> h) noexcept {
        EventLoop::instance().registerFD(listenSocket, h);
    }

    AcceptResult await_resume() const noexcept {
        int fd;
    
        while (true) {
            fd = ::accept(listenSocket, nullptr, nullptr);
    
            if (fd >= 0)
                return {fd, AcceptResult::Status::Ok};
    
            if (errno == EINTR)
                continue;
    
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return {-1, AcceptResult::Status::Retry};
    
            return {-1, AcceptResult::Status::Error};
        }
    }
};

struct RecvResult {
    ssize_t value;
    enum class Status {
        Ok,
        Retry,
        Closed,
        Error
    } status;
};
struct RecvAwaiter {
    int clientSocket;
    void* buffer;
    size_t length;

    RecvAwaiter(int s, void* b, size_t len)
        : clientSocket(s), buffer(b), length(len) {}

    bool await_ready() const noexcept {
        return false;
    }

    void await_suspend(std::coroutine_handle<> h) noexcept {
        EventLoop::instance().registerFD(clientSocket, h);
    }

    RecvResult await_resume() const noexcept {
        while (true) {
            ssize_t n = ::recv(clientSocket, buffer, length, 0);
    
            if (n > 0)
                return {n, RecvResult::Status::Ok};
    
            if (n == 0)
                return {-1, RecvResult::Status::Closed};
    
            if (errno == EINTR)
                continue;
    
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return {-1, RecvResult::Status::Retry};
    
            return {-1, RecvResult::Status::Error};
        }
    }
    };

} // namespace vesper::async