#include <coro.hpp>
#include <utils/debug.hpp>

using namespace std::chrono_literals;

coro::Task<int> foo() {
    debug(), "foo() called\n";
    co_return 42;
}

coro::Task<void> bar() {
    auto value = co_await foo();
    debug(), "bar() called with value: ", value;

    debug(), "bar() sleeping for 2 seconds\n";
    co_await coro::sleepFor(2s);
    debug(), "bar() slept for 2 seconds\n";
    co_return;
}

int main() {
    // scheduler
    auto task = bar();
    while (!task.m_coroutine.done()) {
        task.m_coroutine.resume();
    }
    return 0;
}
