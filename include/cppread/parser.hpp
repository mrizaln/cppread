#ifndef CPPREAD_PARSER_HPP
#define CPPREAD_PARSER_HPP

#include "cppread/common.hpp"

#include <cctype>
#include <array>
#include <charconv>
#include <string>
#include <string_view>

namespace cppread
{
    template <typename>
    struct Parser
    {
        static_assert(false, "No parser specialization for the type");
    };

    // for char
    template <>
    struct Parser<char>
    {
        Result<char> parse(std::string_view str) const noexcept { return str[0]; }
    };

    // for boolean
    template <>
    struct Parser<bool>
    {
        Result<bool> parse(std::string_view str) const noexcept
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

    // for fundamental types
    template <Fundamental T>
    struct Parser<T>
    {
        Result<T> parse(std::string_view str) const noexcept
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

    // for std::string
    template <>
    struct Parser<std::string>
    {
        Result<std::string> parse(std::string_view str) const noexcept
        {
            return std::string{ str.begin(), str.size() };
        }
    };

    template <typename T>
    concept Parseable = requires(const Parser<T> p, std::string_view str) {
        { p.parse(str) } noexcept -> std::same_as<Result<T>>;
    };
}

#endif /* end of include guard: CPPREAD_PARSER_HPP */
