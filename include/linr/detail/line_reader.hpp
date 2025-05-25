#ifndef LINR_READER_HPP
#define LINR_READER_HPP

#include "linr/common.hpp"

#include <concepts>
#include <cstdio>
#include <memory>
#include <utility>
#include <vector>

namespace linr::detail
{
    template <typename L>
    concept Line = requires (const L l) {
        { l.view() } noexcept -> std::same_as<Str>;
    };

    template <typename R>
    concept LineReader = requires (R r) {
        typename R::Line;
        requires Line<typename R::Line>;

        { r.readline() } noexcept -> std::same_as<Opt<typename R::Line>>;
    };

#if defined(__GLIBC__) and defined(LINR_ENABLE_GETLINE)
    struct GetlineReader
    {
        struct Line
        {
            using Ptr = std::unique_ptr<char, decltype(&free)>;

            Line(char* data, std::size_t size) noexcept
                : m_data{ data, &free }
                , m_size{ size }
            {
            }

            Str view() const noexcept { return Str{ m_data.get(), m_size }; }

            Ptr         m_data;
            std::size_t m_size;
        };

        Opt<Line> readline() const noexcept
        {
            char*  line  = nullptr;
            size_t len   = 0;
            auto   nread = getline(&line, &len, stdin);

            if (nread == -1) {
                free(line);
                return {};
            } else if (line[nread - 1] == '\n') {
                // remove trailing newline
                line[nread - 1] = '\0';
            }

            return Opt<Line>{ std::in_place, line, static_cast<std::size_t>(nread) };
        }
    };
    static_assert(LineReader<GetlineReader>);

    struct BufGetlineReader
    {
        struct Line
        {
            Line(char* ptr, std::size_t size)
                : m_str{ ptr, size }
            {
            }
            Str view() const noexcept { return m_str; }
            Str m_str;
        };

        BufGetlineReader(std::size_t size)
            : m_buf{ static_cast<char*>(malloc(size)) }
            , m_size{ size }
        {
        }

        ~BufGetlineReader()
        {
            if (m_buf) {
                free(m_buf);
            }
        }

        BufGetlineReader(BufGetlineReader&& other)
            : m_buf{ std::exchange(other.m_buf, nullptr) }
            , m_size{ std::exchange(other.m_size, 0) }
        {
        }

        BufGetlineReader& operator=(BufGetlineReader&& other)
        {
            if (this == &other) {
                return *this;
            }

            if (m_buf) {
                free(m_buf);
            }

            m_buf  = std::exchange(other.m_buf, nullptr);
            m_size = std::exchange(other.m_size, 0);

            return *this;
        }

        BufGetlineReader(const BufGetlineReader&)            = delete;
        BufGetlineReader& operator=(const BufGetlineReader&) = delete;

        Opt<Line> readline() noexcept
        {
            auto nread = getline(&m_buf, &m_size, stdin);
            if (nread == -1) {
                return {};
            } else if (m_buf[nread - 1] == '\n') {
                // remove trailing newline
                m_buf[nread - 1] = '\0';
            }

            return Opt<Line>{ std::in_place, m_buf, static_cast<std::size_t>(nread) };
        }

        using Ptr = std::unique_ptr<char, decltype(&free)>;

        char*       m_buf  = nullptr;
        std::size_t m_size = 0;
    };
    static_assert(LineReader<BufGetlineReader>);
#endif

    struct FgetsReader
    {
        struct Line
        {
            using Data = std::vector<char>;
            Str  view() const noexcept { return { m_data.data(), m_data.size() }; }
            Data m_data;
        };

        Opt<Line> readline() const noexcept
        {
            auto        line   = Line::Data(256, '\0');
            std::size_t offset = 0;
            bool        first  = true;

            while (true) {
                auto res = std::fgets(line.data() + offset, static_cast<int>(line.size() - offset), stdin);
                if (res == nullptr and first) {
                    return {};
                }

                first = false;

                // fgets encountered newline or EOF
                if (auto last = line[line.size() - 2]; last == '\0' or last == '\n') {
                    break;
                }

                // fgets reached the limit of the buffer; double the size
                offset = line.size() - 1;
                line.resize(line.size() * 2, '\0');
            }

            return Opt<Line>{ std::move(line) };
        }
    };
    static_assert(LineReader<FgetsReader>);

    struct BufFgetsReader
    {
        struct Line
        {
            Line(char* ptr, std::size_t size)
                : m_str{ ptr, size }
            {
            }
            Str view() const noexcept { return m_str; }
            Str m_str;
        };

        BufFgetsReader(std::size_t size)
            : m_buf(size, '\0')
        {
        }

        ~BufFgetsReader() = default;

        BufFgetsReader(BufFgetsReader&&)            = default;
        BufFgetsReader& operator=(BufFgetsReader&&) = default;

        BufFgetsReader(const BufFgetsReader&)            = delete;
        BufFgetsReader& operator=(const BufFgetsReader&) = delete;

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

            return Opt<Line>{ std::in_place, m_buf.data(), m_buf.size() };
        }

        std::vector<char> m_buf;
    };
    static_assert(LineReader<BufFgetsReader>);

#if defined(__GLIBC__) and defined(LINR_ENABLE_GETLINE)
    using Reader    = GetlineReader;
    using BufReader = BufGetlineReader;
#else
    using Reader    = FgetsReader;
    using BufReader = BufFgetsReader;
#endif
}

#endif /* end of include guard: LINR_READER_HPP */
