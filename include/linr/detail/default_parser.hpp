#ifndef LINR_DETAIL_DEFAULT_PARSER_HPP
#define LINR_DETAIL_DEFAULT_PARSER_HPP

#include "linr/common.hpp"

#include <array>
#include <cctype>
#include <charconv>
#include <string>

namespace linr::detail
{
    template <typename>
    struct DefaultParser;

    // specialization for char
    template <>
    struct DefaultParser<char>
    {
        Result<char> parse(Str str) const noexcept { return str[0]; }
    };

    // specialization for boolean
    template <>
    struct DefaultParser<bool>
    {
        Result<bool> parse(Str str) const noexcept
        {
            using Buf = std::array<char, 6>;

            constexpr auto litFalse = Buf{ "false" };
            constexpr auto litTrue  = Buf{ "true" };

            auto buf = Buf{};

            auto size = std::min(str.size(), buf.size());
            for (std::size_t i = 0; i < size; ++i) {
                buf[i] = static_cast<char>(std::tolower(str[i]));
            }

            if (buf[0] == '0') {
                return false;
            } else if (buf[0] == '1') {
                return true;
            }

            if (buf == litFalse) {
                return false;
            } else if (buf == litTrue) {
                return true;
            }

            return Error::InvalidInput;
        }
    };

    // specialization for fundamental types
    template <Fundamental T>
    struct DefaultParser<T>
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
    struct DefaultParser<std::string>
    {
        Result<std::string> parse(Str str) const noexcept { return std::string{ str.begin(), str.size() }; }
    };
}

#endif /* end of include guard: LINR_DETAIL_DEFAULT_PARSER_HPP */
