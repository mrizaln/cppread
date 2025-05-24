#ifndef LINR_READ_HPP
#define LINR_READ_HPP

#include "linr/detail/read.hpp"

namespace linr
{
    /**
     * @brief Read multiple values from stdin.
     *
     * @param prompt The prompt.
     * @param delim Delimiter, only `char` so you can't use unicode.
     */
    template <Parseable... Ts>
        requires(sizeof...(Ts) > 1)
    Results<Ts...> read(Opt<Str> prompt = std::nullopt, char delim = ' ') noexcept
    {
        auto reader = detail::Reader{};
        return detail::read_impl<Ts...>(reader, prompt, delim);
    }

    /**
     * @brief Read a single value from stdin.
     *
     * @param prompt The prompt.
     * @param delim Delimiter, only `char` so you can't use unicode.
     */
    template <Parseable T>
    Result<T> read(Opt<Str> prompt = std::nullopt, char delim = ' ') noexcept
    {
        auto reader = detail::Reader{};
        auto result = detail::read_impl<T>(reader, prompt, delim);
        if (result) {
            return std::get<0>(std::move(result).value());
        }
        return result.error();
    }

    /**
     * @brief Read a string until '\n' is found (aka getline)
     *
     * @param prompt The prompt.
     */
    inline Result<std::string> read(Opt<Str> prompt = std::nullopt) noexcept
    {
        auto reader = detail::Reader{};
        auto result = detail::read_impl<std::string>(reader, prompt, '\n');
        if (result) {
            return std::get<0>(std::move(result).value());
        }
        return result.error();
    }
}

#endif /* end of include guard: LINR_READ_HPP */
