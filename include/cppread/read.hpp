#ifndef CPPREAD_READ_HPP
#define CPPREAD_READ_HPP

#include "cppread/common.hpp"
#include "cppread/parser.hpp"

#include "cppread/util/split.hpp"

// =============================================================================
//  Forward declarations
// =============================================================================

namespace cppread
{
    /**
     * @brief Read multiple values from stdin.
     *
     * @param prompt The prompt.
     * @param delim Delimiter, only `char` so you can't use unicode
     */
    template <Parseable... Ts>
        requires(sizeof...(Ts) > 1)
    Results<Ts...> read(Str prompt, char delim = ' ') noexcept;

    /**
     * @brief Read a single value from stdin.
     *
     * @param prompt The prompt.
     * @param delim Delimiter, only `char` so you can't use unicode.
     */
    template <Parseable T>
    Result<T> read(Str prompt, char delim = ' ') noexcept;

    /**
     * @brief Read a string until '\n' is found (aka getline)
     *
     * @param prompt The prompt.
     */
    inline Result<std::string> read(Str prompt) noexcept;
}

// =============================================================================
//  Implementation
// =============================================================================

#include <string>
#include <cstdio>

namespace cppread::detail
{
    template <Parseable... Ts>
        requires(sizeof...(Ts) >= 1)
    Results<Ts...> read_impl(Str prompt, char delim) noexcept
    {
        constexpr std::size_t N = sizeof...(Ts);

        // first and foremost, check whether stdin available at all
        if (std::ferror(stdin)) {
            return Error::Unknown;
        }

        std::fwrite(prompt.data(), sizeof(Str::value_type), prompt.size(), stdout);

        // get line; no buffering whatsoever so it will be slow
        std::string line = {};
        int         ch   = std::fgetc(stdin);
        while (ch != '\n' and ch != EOF) {
            line.push_back(static_cast<char>(ch));
            ch = std::fgetc(stdin);
        }

        if (std::feof(stdin)) {
            return Error::EndOfFile;
        }

        auto parts = util::split<N>(line, delim);
        if (not parts) {
            return Error::InvalidInput;
        } else {
            return parseIntoTuple<Ts...>(*parts);
        }
    }
}

namespace cppread
{
    template <Parseable... Ts>
        requires(sizeof...(Ts) > 1)
    Results<Ts...> read(Str prompt, char delim) noexcept
    {
        return detail::read_impl<Ts...>(prompt, delim);
    }

    template <Parseable T>
    Result<T> read(Str prompt, char delim) noexcept
    {
        auto result = detail::read_impl<T>(prompt, delim);
        if (result) {
            return std::get<0>(std::move(result).value());
        }
        return result.error();
    }

    inline Result<std::string> read(Str prompt) noexcept
    {
        auto result = detail::read_impl<std::string>(prompt, '\n');
        if (result) {
            return std::get<0>(std::move(result).value());
        }
        return result.error();
    }
}

#endif /* end of include guard: CPPREAD_READ_HPP */
