#pragma once

#include <thread>
#include <queue>
#include <functional>
#include <vector>
#include <mutex>
#include <condition_variable>

// Inspiration: https://stackoverflow.com/questions/15752659/thread-pooling-in-c11

class threadPool {
    private:
        std::queue<std::function<void()>> tasks;
        std::vector<std::thread> threads;
        bool shouldTerminate = false;
        std::mutex tasksMutex;
        std::condition_variable mutexCondition;
        
        void threadLoop();
    public:
        threadPool();
        
        void newTask(std::function<void()> handler);
        void stop();
};