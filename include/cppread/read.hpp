#ifndef CPPREAD_READ_HPP
#define CPPREAD_READ_HPP

#include <array>
#include <optional>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <variant>

namespace cppread
{
    template <typename T>
    concept Fundamental = std::is_fundamental_v<T>;

    enum class Error
    {
        InvalidInput,
        EndOfFile,
        Unknown,    // unknown error, usually platform-specific [check errno]
    };

    template <typename T>
    class Result;

    // read values from stdin
    template <Fundamental... Ts>
        requires(sizeof...(Ts) > 1)
    Result<std::tuple<Ts...>> read(std::string_view prompt, char delim = ' ') noexcept;

    // read a single value from stdin
    template <Fundamental T>
    Result<T> read(std::string_view prompt, char delim = ' ') noexcept;
}

namespace cppread::detail
{
    template <std::size_t N>
    using Split = std::array<std::string_view, N>;

    template <std::size_t N>
    constexpr std::optional<Split<N>> split(std::string_view str, char delim) noexcept;
}

// Implementation

#include <string>
#include <cstdio>
#include <charconv>

namespace cppread
{
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

    template <Fundamental... Ts>
        requires(sizeof...(Ts) >= 1)
    Result<std::tuple<Ts...>> read_impl(std::string_view prompt, char delim) noexcept
    {
        constexpr std::size_t N = sizeof...(Ts);

        std::fwrite(prompt.data(), 1, prompt.size(), stdout);

        std::string line = {};
        int         ch   = std::fgetc(stdin);

        while (ch != '\n' and ch != EOF) {
            line.push_back(static_cast<char>(ch));
            ch = std::fgetc(stdin);
        }

        if (std::feof(stdin)) {
            return Error::EndOfFile;
        } else if (std::ferror(stdin)) {
            return Error::Unknown;
        }

        auto parts = detail::split<N>(line, delim);
        if (!parts) {
            return Error::InvalidInput;
        }

        std::tuple<Ts...>    result = {};
        std::optional<Error> error  = std::nullopt;

        auto parse = [&]<std::size_t I>() {
            // if an error has already occurred, skip parsing
            if (error) {
                return;
            }

            auto str       = parts->at(I);
            auto [ptr, ec] = std::from_chars(str.begin(), str.end(), std::get<I>(result));

            if (ec == std::errc::invalid_argument || ec == std::errc::result_out_of_range) {
                error = Error::InvalidInput;
            }
        };

        auto handler = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            (parse.template operator()<Is>(), ...);
        };

        handler(std::make_index_sequence<N>{});

        if (error) {
            return error.value();
        }

        return result;
    }

    template <Fundamental... Ts>
        requires(sizeof...(Ts) > 1)
    Result<std::tuple<Ts...>> read(std::string_view prompt, char delim) noexcept
    {
        return read_impl<Ts...>(prompt, delim);
    }

    template <Fundamental T>
    Result<T> read(std::string_view prompt, char delim) noexcept
    {
        auto result = read_impl<T>(prompt, delim);
        if (result) {
            return std::get<T>(std::move(result).value());
        }

        return result.error();
    }
}

namespace cppread::detail
{
    template <std::size_t N>
    constexpr std::optional<Split<N>> split(std::string_view str, char delim) noexcept
    {
        Split<N>    result = {};
        std::size_t i      = 0;
        std::size_t j      = 0;

        // in case the delimiter is at the beginning of the string
        while (str[j] == delim) {
            j++;

            if (j == str.size()) {
                return std::nullopt;
            }
        }

        while (i < N and j < str.size()) {
            std::size_t pos = str.find(delim, j);

            // in case multiple delimiters are together
            while (pos == j) {
                j++;

                if (j == str.size()) {
                    return std::nullopt;
                }

                pos = str.find(delim, j);
            }

            if (pos == std::string_view::npos) {
                result[i++] = str.substr(j);
                break;
            }

            result[i++] = str.substr(j, pos - j);
            j           = pos + 1;
        }

        if (i != N) {
            return std::nullopt;
        }

        return result;
    }
}

#endif /* end of include guard: CPPREAD_READ_HPP */
