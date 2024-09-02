#ifndef CPPREAD_READ_HPP
#define CPPREAD_READ_HPP

#include "cppread/common.hpp"
#include "cppread/parser.hpp"

#include "cppread/detail/for_each_tuple.hpp"
#include "cppread/detail/split.hpp"

#include <string_view>
#include <tuple>

// =============================================================================
//  Forward declarations
// =============================================================================

namespace cppread
{
    /*
     * read multiple values from stdin
     */
    template <Parseable... Ts>
        requires(sizeof...(Ts) > 1)
    Result<std::tuple<Ts...>> read(std::string_view prompt, char delim = ' ') noexcept;

    /*
     * read a single value from stdin
     */
    template <Parseable T>
    Result<T> read(std::string_view prompt, char delim = ' ') noexcept;
}

// =============================================================================
//  Implementation
// =============================================================================

#include <string>
#include <cstdio>

namespace cppread
{
    template <Parseable... Ts>
        requires(sizeof...(Ts) >= 1)
    Result<std::tuple<Ts...>> read_impl(std::string_view prompt, char delim) noexcept
    {
        using Ret               = std::tuple<Ts...>;
        constexpr std::size_t N = sizeof...(Ts);

        std::fwrite(prompt.data(), sizeof(std::string_view::value_type), prompt.size(), stdout);

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

        Ret                  result = {};
        std::optional<Error> error  = std::nullopt;

        detail::forEachTuple(result, [&]<std::size_t I, typename T>(T& value) {
            if (error) {
                return;    // skip parse if error has happened before
            }

            auto parser = Parser<T>{};
            if (auto parsed = parser.parse(parts->at(I)); parsed) {
                value = std::move(parsed).value();
            } else {
                error = parsed.error();
            }
        });

        if (error) {
            return error.value();
        }

        return result;
    }

    template <Parseable... Ts>
        requires(sizeof...(Ts) > 1)
    Result<std::tuple<Ts...>> read(std::string_view prompt, char delim) noexcept
    {
        return read_impl<Ts...>(prompt, delim);
    }

    template <Parseable T>
    Result<T> read(std::string_view prompt, char delim) noexcept
    {
        auto result = read_impl<T>(prompt, delim);
        if (result) {
            return std::get<0>(std::move(result).value());
        }
        return result.error();
    }
}

#endif /* end of include guard: CPPREAD_READ_HPP */
