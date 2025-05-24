#ifndef LINR_COMMON_HPP
#define LINR_COMMON_HPP

#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

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

    /**
     * @brief Result class that store parsed value or an error
     */
    template <typename T>
    class [[nodiscard]] Result
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

        bool is_value() const noexcept { return static_cast<bool>(*this); }
        bool is_error() const noexcept { return not is_value(); }

        T&&      value() && noexcept(false) { return std::move(std::get<T>(m_value)); }
        T&       value() & noexcept(false) { return std::get<T>(m_value); }
        const T& value() const& noexcept(false) { return std::get<T>(m_value); }

        T value_or(T&& defaultt) && noexcept { return *this ? std::move(std::get<T>(m_value)) : defaultt; }
        const T& value_or(T&& defaultt) const& noexcept { return *this ? std::get<T>(m_value) : defaultt; }

        Error&       error() noexcept(false) { return std::get<Error>(m_value); }
        const Error& error() const noexcept(false) { return std::get<Error>(m_value); }

        std::variant<T, Error>& variant() noexcept { return m_value; }

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
    concept RepeatFn = requires(const Fn fn, T t, Error error) {
        { fn(t) } -> std::same_as<bool>;
        { fn(error) } -> std::same_as<Opt<T>>;
    };
}

#endif /* end of include guard: LINR_COMMON_HPP */
