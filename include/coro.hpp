#include <chrono>
#include <coroutine>
#include <exception>
#include <memory>
#include <thread>
#include <utility>

namespace coro {

struct PreviousAwaiter {
    bool await_ready() const noexcept {
        return false;
    }

    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<> coroutine) const noexcept {
        if (m_previous) {
            return m_previous;
        } else {
            return std::noop_coroutine();
        }
    }

    void await_resume() const noexcept {}

    std::coroutine_handle<> m_previous;
};

template <typename T>
struct Promise {
    auto get_return_object() noexcept {
        return std::coroutine_handle<Promise>::from_promise(*this);
    }

    auto initial_suspend() const noexcept {
        return std::suspend_always{};
    }

    auto final_suspend() const noexcept {
        return PreviousAwaiter{m_previous};
    }

    void unhandled_exception() {
        m_exception = std::current_exception();
    }

    void return_value(T value) noexcept {
        std::construct_at(&m_value, std::move(value));
    }

    T result() {
        if (m_exception) {
            std::rethrow_exception(m_exception);
        }
        return m_value;
    }

    ~Promise() {
        std::destroy_at(&m_value);
        if (m_exception) {
            std::rethrow_exception(m_exception);
        }
    }

    union {
        T m_value;
    };

    std::exception_ptr m_exception{};

    std::coroutine_handle<> m_previous{};
};

template <>
struct Promise<void> {
    auto get_return_object() noexcept {
        return std::coroutine_handle<Promise>::from_promise(*this);
    }

    auto initial_suspend() const noexcept {
        return std::suspend_always{};
    }

    auto final_suspend() const noexcept {
        return PreviousAwaiter{m_previous};
    }

    void unhandled_exception() {
        m_exception = std::current_exception();
    }

    void return_void() const noexcept {}

    void result() const {
        if (m_exception) {
            std::rethrow_exception(m_exception);
        }
    }

    std::exception_ptr m_exception{};
    std::coroutine_handle<> m_previous{};
};

template <typename T = void, typename P = Promise<T>>
struct [[nodiscard("Task must be co_await to execute")]] Task {
    using promise_type = P;

    Task(std::coroutine_handle<promise_type> coroutine = nullptr) noexcept
        : m_coroutine(coroutine) {}

    Task(Task&& other) noexcept : m_coroutine(other.m_coroutine) {
        other.m_coroutine = nullptr;
    }

    Task& operator=(Task&& other) noexcept {
        std::swap(m_coroutine, other.m_coroutine);
        return *this;
    }

    ~Task() {
        if (m_coroutine) {
            m_coroutine.destroy();
        }
    }

    struct Awaiter {
        bool await_ready() const noexcept {
            return false;
        }

        std::coroutine_handle<>
        await_suspend(std::coroutine_handle<> coroutine) noexcept {
            m_coroutine.promise().m_previous = coroutine;
            return m_coroutine;
        }

        auto await_resume() const {
            return m_coroutine.promise().result();
        }

        std::coroutine_handle<promise_type> m_coroutine;
    };

    auto operator co_await() const noexcept {
        return Awaiter{m_coroutine};
    }

    std::coroutine_handle<promise_type> m_coroutine;
};

struct SleepAwaiter {
    bool await_ready() const {
        return std::chrono::system_clock::now() >= m_expireTime;
    }

    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<> coroutine) const noexcept {
        return m_expireTime > std::chrono::system_clock::now()
                   ? coroutine
                   : std::noop_coroutine();
    }

    std::chrono::system_clock::time_point m_expireTime;
};

inline Task<void> sleepFor(std::chrono::system_clock::duration duration) {
    std::this_thread::sleep_for(duration);
    co_return;
}

inline Task<void> sleepUntil(std::chrono::system_clock::time_point timePoint) {
    std::this_thread::sleep_until(timePoint);
    co_return;
}

} // namespace coro
