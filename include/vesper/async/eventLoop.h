#pragma once
#include <coroutine>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdexcept>

#include "eventLoop_fwd.h"
#include "../utils/logging.h"
#include "../utils/threadPool.h"

namespace vesper::async {

class EventLoop {
public:
    #define MAX_EVENTS 64

    static EventLoop& instance() {
        static EventLoop loop;
        return loop;
    }

    struct CoroutineObject {
        std::coroutine_handle<> h;
    };

    void registerFD(int fd, std::coroutine_handle<> h) {
        auto* ptr = new CoroutineObject{h};

        epoll_event ev{};
        ev.events = EPOLLIN | EPOLLONESHOT;
        ev.data.ptr = ptr;

        if (epoll_ctl(epollFD, EPOLL_CTL_ADD, fd, &ev) == -1) {
            if (errno == EEXIST) {
                if (epoll_ctl(epollFD, EPOLL_CTL_MOD, fd, &ev) == -1) {
                    delete ptr;
                    log(LogType::Error, "epoll_ctl MOD failed");
                }
            } else {
                delete ptr;
                log(LogType::Error, "epoll_ctl ADD failed");
            }
        }
    }

    void loop() {
        epoll_event events[MAX_EVENTS];
        threadPool threads{0};

        while (true) {
            int n = epoll_wait(epollFD, events, MAX_EVENTS, -1);

            if (n == -1) {
                if (errno == EINTR)
                    continue;

                log(LogType::Error, "epoll_wait failed");
                continue;
            }

            for (int i = 0; i < n; i++) {
                auto* obj =
                    static_cast<CoroutineObject*>(events[i].data.ptr);

                if (!obj)
                    continue;

                auto h = obj->h;

                delete obj;

                if (!h || h.done())
                    continue;

                threads.newTask([h]() mutable {
                    h.resume();
                });
            }
        }
    }

private:
    int epollFD;

    EventLoop() {
        epollFD = epoll_create1(0);

        if (epollFD == -1)
            log(LogType::Error, "epoll_create1 failed");
    }
};

}