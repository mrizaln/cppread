#ifndef CPPREAD_PARSER_HPP
#define CPPREAD_PARSER_HPP

#include "cppread/common.hpp"
#include "cppread/util/for_each_tuple.hpp"

#include <array>
#include <cctype>
#include <charconv>
#include <span>
#include <string>

namespace cppread
{
    template <typename>
    struct Parser
    {
        static_assert(false, "No Parser specialization for the type");
    };

    // specialization for char
    template <>
    struct Parser<char>
    {
        Result<char> parse(Str str) const noexcept { return str[0]; }
    };

    // specialization for boolean
    template <>
    struct Parser<bool>
    {
        Result<bool> parse(Str str) const noexcept
        {
            using Buf = std::array<char, 6>;
            Buf buf   = {};

            auto size = std::min(str.size(), buf.size());
            for (std::size_t i = 0; i < size; ++i) {
                buf[i] = static_cast<char>(std::tolower(str[i]));
            }

            if (buf[0] == '0') {
                return false;
            } else if (buf[0] == '1') {
                return true;
            }

            if (buf == Buf{ "false" }) {
                return false;
            } else if (buf == Buf{ "true" }) {
                return true;
            }

            return Error::InvalidInput;
        }
    };

    // specialization for fundamental types
    template <Fundamental T>
    struct Parser<T>
    {
        Result<T> parse(Str str) const noexcept
        {
            T value;
            auto [ptr, ec] = std::from_chars(str.begin(), str.end(), value);

            if (ec == std::errc::invalid_argument) {
                return Error::InvalidInput;
            } else if (ec == std::errc::result_out_of_range) {
                return Error::OutOfRange;
            }

            return value;
        }
    };

    // specialization for std::string
    template <>
    struct Parser<std::string>
    {
        Result<std::string> parse(Str str) const noexcept { return std::string{ str.begin(), str.size() }; }
    };

    template <typename T>
    concept Parseable = requires(const Parser<T> p, Str str) {
        { p.parse(str) } noexcept -> std::same_as<Result<T>>;
    };

    /**
     * @brief Helper function that calls the specialized `Parser` member function, since writing
     *        `cppread::Parser<T>{}.parse(str)` will get old really quickly.
     */
    template <Parseable T>
    Result<T> parse(Str str) noexcept
    {
        return Parser<T>{}.parse(str);
    }

    /**
     * @brief Helper function that parse span of str directly into tuple
     *
     * @tparam Ts The types to parse.
     * @param values Span of string views, the extent matches the number of types.
     * @return The resulting parsed values as tuple or an error.
     */
    template <typename... Ts>
    constexpr Results<Ts...> parseIntoTuple(std::span<Str, sizeof...(Ts)> values) noexcept
    {
        using Seq = std::index_sequence_for<Ts...>;

        // parse into tuple of Result objects
        const auto multiparse = [&]<std::size_t... Is>(std::index_sequence<Is...>) -> Tup<Result<Ts>...> {
            return { parse<Ts>(values[Is])... };
        };
        auto maybeResult = multiparse(Seq{});

        // check whether any of the values is an error
        auto error = Opt<Error>{};
        util::forEachTuple(maybeResult, [&]<std::size_t I, typename T>(T& value) {
            if (not error.has_value() and value.is_error()) {
                error = value.error();
            }
        });
        if (error.has_value()) {
            return error.value();
        }

        // if no error, flatten the tuple and return
        const auto flatten = [&]<std::size_t... Is>(std::index_sequence<Is...>) -> Tup<Ts...> {
            return { std::move(std::get<Is>(maybeResult)).value()... };
        };
        return flatten(Seq{});
    }
}

#endif /* end of include guard: CPPREAD_PARSER_HPP */
