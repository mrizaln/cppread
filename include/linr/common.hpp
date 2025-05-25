#ifndef LINR_COMMON_HPP
#define LINR_COMMON_HPP

#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

#if defined(__cpp_lib_expected)
#    include <expected>
#endif

namespace linr
{
    template <typename T>
    concept Fundamental = std::is_fundamental_v<T>;

    using Str = std::string_view;

    template <typename... Ts>
    using Tup = std::tuple<Ts...>;

    template <typename T>
    using Opt = std::optional<T>;

    template <typename... Ts>
    using Opts = std::optional<Tup<Ts...>>;

    enum class Error : char
    {
        // generic error
        InvalidInput = 0b0001,    // `failbit`; generic parse failure (eg: parsing "asd" to `int`)
        OutOfRange   = 0b0010,    // `failbit`; parsed value can't be contained within given type

        // stream error, unrecoverable
        EndOfFile = 0b0101,    // `eofbit`; EOF reached
        Unknown   = 0b0110,    // `badbit`; unknown error, usually platform-specific [check errno]
    };

    inline Str toString(Error error)
    {
        switch (error) {
            // clang-format off
        case Error::InvalidInput:   return "Invalid input (failed to parse input)";
        case Error::OutOfRange:     return "Parsed value can't be contained within given type";
        case Error::EndOfFile:      return "stdin EOF has been reached";
        case Error::Unknown:        return "Unknown error (platform error)";
        default:                    return "Unknown error";
            // clang-format on
        }
    }

#if defined(__cpp_lib_expected)
    template <typename T>
    using Result = std::expected<T, Error>;
#else
    /**
     * @brief Result class that store parsed value or an error
     */
    template <typename T>
    class [[nodiscard]] Result
    {
    public:
        Result() = default;

        template <typename... Args>
            requires std::constructible_from<T, Args...>
        Result(Args&&... args) noexcept
            : m_value{ std::in_place_type<T>, std::forward<Args>(args)... }
        {
        }

        Result(Error error) noexcept
            : m_value{ error }
        {
        }

        T&&      value() && { return std::get<T>(std::move(m_value)); }
        T&       value() & { return std::get<T>(m_value); }
        const T& value() const& { return std::get<T>(m_value); }

        T value_or(T&& defaultt) && noexcept { return *this ? std::move(std::get<T>(m_value)) : defaultt; }
        const T& value_or(T&& defaultt) const& noexcept { return *this ? std::get<T>(m_value) : defaultt; }

        Error&&      error() && { return std::get<Error>(std::move(m_value)); }
        Error&       error() & { return std::get<Error>(m_value); }
        const Error& error() const& { return std::get<Error>(m_value); }

        bool has_value() const noexcept { return std::holds_alternative<T>(m_value); }

        explicit operator bool() const noexcept { return has_value(); }

        T&&      operator*() && noexcept { return std::move(value()); }
        T&       operator*() & noexcept { return value(); }
        const T& operator*() const& noexcept { return value(); }

        T*       operator->() noexcept { return &value(); }
        const T* operator->() const noexcept { return &value(); }

    private:
        std::variant<T, Error> m_value;
    };

    template <typename... Ts>
    using Results = Result<Tup<Ts...>>;

    /**
     * @brief Decides whether `Fn` is a suitable callable object for `linr::readRepeat` callback
     *
     * @tparam Fn The callable object
     * @tparam T The parsed object if it's successful
     */
    template <typename Fn, typename T>
    concept RepeatFn = requires (const Fn fn, T t, Error error) {
        { fn(t) } -> std::same_as<bool>;
        { fn(error) } -> std::same_as<Opt<T>>;
    };
#endif

    template <typename T, typename... Args>
    Result<T> make_result(Args&&... args)
    {
#if defined(__cpp_lib_expected)
        return Result<T>{ std::in_place, std::forward<Args>(args)... };
#else
        return Result<T>{ std::forward<Args>(args)... };
#endif
    }

    template <typename T>
    Result<T> make_error(Error error)
    {
#if defined(__cpp_lib_expected)
        return Result<T>{ std::unexpect, error };
#else
        return Result<T>{ error };
#endif
    }
}

#endif /* end of include guard: LINR_COMMON_HPP */
