#ifndef CPPREAD_COMMON_HPP
#define CPPREAD_COMMON_HPP

#include <type_traits>
#include <utility>
#include <variant>

namespace cppread
{
    template <typename T>
    concept Fundamental = std::is_fundamental_v<T>;

    enum class Error
    {
        InvalidInput,
        OutOfRange,
        EndOfFile,
        Unknown,    // unknown error, usually platform-specific [check errno]
    };

    template <typename T>
    class Result
    {
    public:
        template <typename TT>
        Result(TT&& value) noexcept
            : m_value(std::forward<TT>(value))
        {
        }

        Result(Error error) noexcept
            : m_value(error)
        {
        }

        explicit operator bool() const noexcept { return std::holds_alternative<T>(m_value); }

        T&&      value() && noexcept(false) { return std::move(std::get<T>(m_value)); }
        T&       value() & noexcept(false) { return std::get<T>(m_value); }
        const T& value() const& noexcept(false) { return std::get<T>(m_value); }

        Error error() const noexcept(false) { return std::get<Error>(m_value); }

        std::variant<T, Error>& variant() noexcept { return m_value; }

    private:
        std::variant<T, Error> m_value;
    };
}

#endif /* end of include guard: CPPREAD_COMMON_HPP */
