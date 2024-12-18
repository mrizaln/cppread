#ifndef CPPREAD_BUF_READER_HPP
#define CPPREAD_BUF_READER_HPP

#include "cppread/common.hpp"
#include "cppread/parser.hpp"
#include "cppread/detail/read.hpp"

#include <algorithm>

namespace cppread
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
            requires(sizeof...(Ts) > 1)
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

#endif /* end of include guard: CPPREAD_BUF_READER_HPP */
