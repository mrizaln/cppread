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
        requires (sizeof...(Ts) > 1) and (std::movable<Ts> and ...)
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
        requires std::movable<T>
    Result<T> read(Opt<Str> prompt = std::nullopt, char delim = ' ') noexcept
    {
        auto reader = detail::Reader{};
        auto result = detail::read_impl<T>(reader, prompt, delim);
        if (result) {
            return make_result<T>(std::get<0>(std::move(result).value()));
        }
        return make_error<T>(result.error());
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
            return make_result<std::string>(std::get<0>(std::move(result).value()));
        }
        return make_error<std::string>(result.error());
    }
}

#endif /* end of include guard: LINR_READ_HPP */
