#ifndef LINR_BUF_READER_HPP
#define LINR_BUF_READER_HPP

#include "linr/common.hpp"
#include "linr/detail/read.hpp"
#include "linr/parser.hpp"

#include <algorithm>

namespace linr
{
    class BufReader
    {
    public:
        struct Line
        {
            Str view() const noexcept { return m_str; }
            Str m_str;
        };

        BufReader(std::size_t size) noexcept
            : m_reader{ size }
        {
        }

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
            return detail::read_impl<Ts...>(m_reader, prompt, delim);
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
            auto result = detail::read_impl<T>(m_reader, prompt, delim);
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
        Result<std::string> read(Opt<Str> prompt = std::nullopt) noexcept
        {
            auto result = detail::read_impl<std::string>(m_reader, prompt, '\n');
            if (result) {
                return std::get<0>(std::move(result).value());
            }
            return result.error();
        }

    private:
        detail::BufReader m_reader;
    };
}

#endif /* end of include guard: LINR_BUF_READER_HPP */
