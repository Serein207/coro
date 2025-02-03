#pragma once

#include <system_error>
#include <type_traits>

namespace coro {

template <typename T>
class Result {
    static_assert(!std::is_reference_v<T>, "T must not be a reference type");

protected:
    std::error_category const * m_error = nullptr;

    union {
        T m_value;
        int m_error_code;
    };
};

} // namespace coro
