#pragma once
#include <coroutine>
#include <exception>

// https://www.wholetomato.com/blog/cpp-coroutines-async-development/

namespace vesper::async {
    
struct Task {
    struct promise_type;
    
    using handle_type = std::coroutine_handle<promise_type>;

    handle_type h;

    Task(handle_type h) : h(h) {}
    
    struct promise_type {
        Task get_return_object() { return Task{handle_type::from_promise(*this)}; }
        
        std::suspend_never initial_suspend() { return {}; }

        std::suspend_never final_suspend() noexcept { return {}; }
    
        void return_void() {}
    
        void unhandled_exception() { std::terminate(); }
    };
};
} // namespace vesper::async