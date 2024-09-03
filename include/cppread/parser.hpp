#ifndef CPPREAD_PARSER_HPP
#define CPPREAD_PARSER_HPP

#include "cppread/common.hpp"

#include <array>
#include <cctype>
#include <charconv>
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
    Result<T> parse(Str str)
    {
        return Parser<T>{}.parse(str);
    }
}

#endif /* end of include guard: CPPREAD_PARSER_HPP */
