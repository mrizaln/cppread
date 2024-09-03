#ifndef CPPREAD_COMMON_HPP
#define CPPREAD_COMMON_HPP

#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

namespace cppread
{
    template <typename T>
    concept Fundamental = std::is_fundamental_v<T>;

    template <typename Fn, typename Ret, typename... Args>
    concept Callable = std::invocable<Fn, Args...> and std::same_as<Ret, std::invoke_result_t<Fn, Args...>>;

    enum class Error
    {
        InvalidInput,    // `failbit`; generic parse failure (eg: parsing "asd" to `int`)
        OutOfRange,      // parsed value can't be contained in given type (eg: value > 2^31-1 for `int`)
        EndOfFile,       // `eofbit`; EOF reached
        Unknown,         // `badbit`; unknown error, usually platform-specific [check errno]
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

    template <typename... Ts>
    using Tuple = std::tuple<Ts...>;

    template <typename... Ts>
    using Results = Result<Tuple<Ts...>>;

    using Str = std::string_view;
}

#endif /* end of include guard: CPPREAD_COMMON_HPP */
