#ifndef CPPREAD_READ_HPP
#define CPPREAD_READ_HPP

#include "cppread/common.hpp"
#include "cppread/parser.hpp"
#include "cppread/util.hpp"

// =============================================================================
//  Forward declarations
// =============================================================================

namespace cppread
{
    /**
     * @brief Read multiple values from stdin.
     *
     * @param prompt The prompt.
     * @param delim Delimiter, only `char` so you can't use unicode.
     */
    template <Parseable... Ts>
        requires(sizeof...(Ts) > 1)
    Results<Ts...> read(Opt<Str> prompt = std::nullopt, char delim = ' ') noexcept;

    /**
     * @brief Read a single value from stdin.
     *
     * @param prompt The prompt.
     * @param delim Delimiter, only `char` so you can't use unicode.
     */
    template <Parseable T>
    Result<T> read(Opt<Str> prompt = std::nullopt, char delim = ' ') noexcept;

    /**
     * @brief Read a string until '\n' is found (aka getline)
     *
     * @param prompt The prompt.
     */
    inline Result<std::string> read(Opt<Str> prompt = std::nullopt) noexcept;
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
    Results<Ts...> read_impl(Opt<Str> prompt, char delim) noexcept
    {
        constexpr std::size_t N = sizeof...(Ts);

        // first and foremost, check whether stdin available at all
        if (std::ferror(stdin)) {
            return Error::Unknown;
        }

        if (prompt) {
            std::fwrite(prompt->data(), sizeof(Str::value_type), prompt->size(), stdout);
        }

        auto line = util::readLine();
        if (not line) {
            return Error::EndOfFile;
        }

        auto parts = util::split<N>(line->view(), delim);
        if (parts) {
            return parseIntoTuple<Ts...>(*parts);
        }
        return Error::InvalidInput;
    }
}

namespace cppread
{
    template <Parseable... Ts>
        requires(sizeof...(Ts) > 1)
    Results<Ts...> read(Opt<Str> prompt, char delim) noexcept
    {
        return detail::read_impl<Ts...>(prompt, delim);
    }

    template <Parseable T>
    Result<T> read(Opt<Str> prompt, char delim) noexcept
    {
        auto result = detail::read_impl<T>(prompt, delim);
        if (result) {
            return std::get<0>(std::move(result).value());
        }
        return result.error();
    }

    inline Result<std::string> read(Opt<Str> prompt) noexcept
    {
        auto result = detail::read_impl<std::string>(prompt, '\n');
        if (result) {
            return std::get<0>(std::move(result).value());
        }
        return result.error();
    }
}

#endif /* end of include guard: CPPREAD_READ_HPP */
