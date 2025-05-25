#ifndef LINR_COMMON_HPP
#define LINR_COMMON_HPP

#include <cstdint>
#include <optional>
#include <string_view>
#include <utility>
#include <variant>

#if defined(__cpp_lib_expected)
#    include <expected>
#endif

namespace linr
{
    using Str = std::string_view;

    template <typename... Ts>
    using Tup = std::tuple<Ts...>;

    template <typename T>
    using Opt = std::optional<T>;

    template <typename... Ts>
    using Opts = std::optional<Tup<Ts...>>;

    /**
     * @brief Represent reading and parsing error.
     */
    enum class Error : std::uint8_t
    {
        // parse error
        InvalidInput = 0b0001,    // `failbit`; generic parse failure (eg: parsing "asd" to `int`)
        OutOfRange   = 0b0010,    // `failbit`; integer can't fit in a type

        // stream error, unrecoverable
        EndOfFile = 0b0101,    // `eofbit`; EOF reached, stdin closed
        Unknown   = 0b0110,    // `badbit`; unknown error, usually platform-specific [check errno]
    };

    /**
     * @brief Get error description.
     *
     * @param error The error value.
     * @return Description of the error (static string).
     */
    inline Str to_string(Error error) noexcept
    {
        // clang-format off
        switch (error) {
        case Error::InvalidInput:   return "Invalid input (failed to parse input)";
        case Error::OutOfRange:     return "Parsed value can't be contained within given type";
        case Error::EndOfFile:      return "stdin EOF has been reached";
        case Error::Unknown:        return "Unknown error (platform error, maybe check errno)";
        }
        // clang-format on

        return "Unknown error";
    }

    /**
     * @brief Check if error is stream error.
     *
     * @param error The error value.
     */
    inline bool is_stream_error(Error error) noexcept
    {
        return error == Error::EndOfFile or error == Error::Unknown;
    }

    /**
     * @brief Check if error is parse error.
     *
     * @param error The error value.
     */
    inline bool is_parse_error(Error error) noexcept
    {
        return not is_stream_error(error);
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
#endif

    template <typename... Ts>
    using Results = Result<Tup<Ts...>>;

    template <typename T, typename... Args>
    Result<T> make_result(Args&&... args) noexcept
    {
#if defined(__cpp_lib_expected)
        return Result<T>{ std::in_place, std::forward<Args>(args)... };
#else
        return Result<T>{ std::forward<Args>(args)... };
#endif
    }

    template <typename T>
    Result<T> make_error(Error error) noexcept
    {
#if defined(__cpp_lib_expected)
        return Result<T>{ std::unexpect, error };
#else
        return Result<T>{ error };
#endif
    }
}

#endif /* end of include guard: LINR_COMMON_HPP */
