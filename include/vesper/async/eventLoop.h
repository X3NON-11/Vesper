#pragma once
#include <coroutine>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

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

    void registerFD(int fd, std::coroutine_handle<> h) {
        std::lock_guard lock(mtx);
    
        sockMap[fd] = h;
    
        struct epoll_event ev{};
        ev.events = EPOLLIN;
        ev.data.fd = fd;
    
        if (epoll_ctl(epollFD, EPOLL_CTL_ADD, fd, &ev) == -1) {
            if (errno == EEXIST) {
                epoll_ctl(epollFD, EPOLL_CTL_MOD, fd, &ev);
            }
        }
    }

    void loop() {
        struct epoll_event events[MAX_EVENTS];
        threadPool threads{0};

        while (true) {
            int n = epoll_wait(epollFD, events, MAX_EVENTS, -1);
    
            if (n == -1) {
                if (errno == EINTR) continue;
                log(LogType::Error, "epoll_wait failed");
                continue;
            }
    
            for (int i = 0; i < n; i++) {
                int fd = events[i].data.fd;
            
                std::coroutine_handle<> h;
                
                {
                    std::lock_guard lock(mtx);
                
                    auto it = sockMap.find(fd);
                    if (it == sockMap.end()) continue;
                
                    h = it->second;
                    sockMap.erase(it);
                }
                
                if (h && !h.done()) {
                    threads.newTask([h] {
                        h.resume();
                    });
                }
            }
        }
    }

private:
    std::mutex mtx;
    std::unordered_map<int, std::coroutine_handle<>> sockMap; // fd, coroutine_handle
    int epollFD;
    
    EventLoop() {
        epollFD = epoll_create1(0);
        if (epollFD == -1) {
            log(LogType::Error, "epoll_create1 failed");
        }
    }

    ~EventLoop() {
        
    }
};

}