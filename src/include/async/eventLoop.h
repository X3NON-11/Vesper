#pragma once
#include <coroutine>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdexcept>

#include "eventLoop_fwd.h"

namespace vesper::async {

class EventLoop {
public:
    static constexpr int MAX_EVENTS = 64;

    static EventLoop& instance() {
        static EventLoop loop;
        return loop;
    }

    void watchRead(int fd, std::coroutine_handle<> h) {
        epoll_event ev{};
        ev.events = EPOLLIN;
        ev.data.ptr = h.address();
        epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    }

    void watchWrite(int fd, std::coroutine_handle<> h) {
        epoll_event ev{};
        ev.events = EPOLLOUT | EPOLLET;
        ev.data.ptr = h.address();
        int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
        
        if (ret < 0) {
            log(LogType::Warn, "epoll_ctl watchWrite");
        }
    }
    
    void loop() {
        while (true) {
            int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
            for (int i = 0; i < n; ++i) {
                auto h = std::coroutine_handle<>::from_address(events[i].data.ptr);
                h.resume();
            }
        }
    }

private:
    EventLoop() {
        epfd = epoll_create1(0);
        if (epfd < 0)
            throw std::runtime_error("epoll_create1 failed");
    }

    ~EventLoop() {
        close(epfd);
    }

    int epfd;
    epoll_event events[MAX_EVENTS];
};

}