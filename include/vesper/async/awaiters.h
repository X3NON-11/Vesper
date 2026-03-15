#pragma once 
#include <coroutine> 
#include <sys/socket.h> 
#include <unistd.h> 
#include "eventLoop.h"

namespace vesper::async { 
    struct AcceptAwaiter { 
        int listenFd;
        
        bool await_ready() const noexcept { return false; } 
        void await_suspend(std::coroutine_handle<> h) { EventLoop::instance().watchRead(listenFd, h); } 
        int await_resume() { 
            EventLoop::instance().unwatch(listenFd);
            return accept(listenFd, nullptr, nullptr); 
        } 
    }; 
    
    struct RecvAwaiter { 
        int fd; 
        char* buf; 
        size_t len;
         
        bool await_ready() const noexcept { return false; } 
        void await_suspend(std::coroutine_handle<> h) { EventLoop::instance().watchRead(fd, h); } 
        int await_resume() { 
            EventLoop::instance().unwatch(fd);
            return recv(fd, buf, len, 0); 
        } 
    };
}