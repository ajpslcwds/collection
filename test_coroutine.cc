#include <coroutine>
#include <iostream>

template<typename T>
struct Generator {
    struct promise_type {
        T value;
        std::suspend_always yield_value(T val) {
            value = val;
            return {};
        }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
        Generator get_return_object() { return Generator{*this}; }
    };

    struct Iterator {
        std::coroutine_handle<promise_type> coro;
        bool operator!=(const Iterator& other) const { return !coro.done(); }
        void operator++() { coro.resume(); }
        T operator*() const { return coro.promise().value; }
    };

    Iterator begin() { 
        coro.resume();
        return {coro};
    }
    Iterator end() { return {coro}; }

    std::coroutine_handle<promise_type> coro;
};

Generator<int> range(int n) {
    for (int i = 0; i < n; ++i) {
        co_yield i;
    }
}

int main() {
    for (int i : range(5)) {
        std::cout << i << " "; // 输出: 0 1 2 3 4
    }
    return 0;
}