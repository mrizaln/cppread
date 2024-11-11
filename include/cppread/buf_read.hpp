#ifndef CPPREAD_BUF_READER_HPP
#define CPPREAD_BUF_READER_HPP

#include "cppread/common.hpp"
#include "cppread/parser.hpp"
#include "cppread/detail/read.hpp"

#include <algorithm>
#include <vector>

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
            : m_buf(size, '\0')
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
            return detail::read_impl<Ts...>(*this, prompt, delim);
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
            auto result = detail::read_impl<T>(*this, prompt, delim);
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
            auto result = detail::read_impl<std::string>(*this, prompt, '\n');
            if (result) {
                return std::get<0>(std::move(result).value());
            }
            return result.error();
        }

        /**
         * @brief Read a line from stdin.
         */
        Opt<Line> readline() noexcept
        {
            std::ranges::fill(m_buf, '\0');

            std::size_t offset = 0;
            bool        first  = true;
            while (true) {
                auto res = std::fgets(m_buf.data() + offset, static_cast<int>(m_buf.size() - offset), stdin);
                if (res == nullptr and first) {
                    return {};
                }

                first = false;

                // fgets encountered newline or EOF
                if (auto last = m_buf[m_buf.size() - 2]; last == '\0' or last == '\n') {
                    break;
                }

                // fgets reached the limit of the buffer; double the size
                offset = m_buf.size() - 1;
                m_buf.resize(m_buf.size() * 2, '\0');
            }

            return Opt<Line>{ Str{ m_buf.data(), m_buf.size() } };
        }

    private:
        std::vector<char> m_buf;
    };

    static_assert(detail::LineReader<BufReader>);
}

#endif /* end of include guard: CPPREAD_BUF_READER_HPP */
