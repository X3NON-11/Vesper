#include "../include/vesper/utils/threadPool.h"

threadPool::threadPool() {
    // Max available threads
    const uint32_t numThreads = std::thread::hardware_concurrency();
    for (uint32_t i = 0; i < numThreads; i++) {
        threads.emplace_back(std::thread(&threadPool::threadLoop, this));
    }
}

void threadPool::newTask(std::function<void()> handler) {
    std::unique_lock<std::mutex> lock(tasksMutex);
    tasks.push(handler);
    mutexCondition.notify_one();
}

void threadPool::stop() {
    // These brackets are necessary to define when lock goes out of scope
    {
        std::unique_lock<std::mutex> lock(tasksMutex);
        shouldTerminate = true;
    }
    mutexCondition.notify_all();
    for (std::thread &activeThread : threads) {
        activeThread.join();
    }
    threads.clear();
}

void threadPool::threadLoop() {
    while (true) {
        std::function<void()> task;
        std::unique_lock<std::mutex> lock(tasksMutex);
        mutexCondition.wait(
            lock, [this] { return !tasks.empty() || shouldTerminate; });
        if (shouldTerminate)
            return;
        task = tasks.front();
        tasks.pop();
        task();
    }
}