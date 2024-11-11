#ifndef CPPREAD_READER_HPP
#define CPPREAD_READER_HPP

#include "cppread/common.hpp"

#include <concepts>
#include <memory>
#include <vector>

namespace cppread::detail
{
    template <typename L>
    concept Line = requires(const L l) {
        { l.view() } noexcept -> std::same_as<Str>;
    };

    template <typename R>
    concept LineReader = requires(R r) {
        typename R::Line;
        requires Line<typename R::Line>;

        { r.readline() } noexcept -> std::same_as<Opt<typename R::Line>>;
    };

#if defined(__GLIBC__) and defined(CPPREAD_ENABLE_GETLINE)
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

#if defined(__GLIBC__) and defined(CPPREAD_ENABLE_GETLINE)
    using NoBufReader = GetlineReader;
#else
    using NoBufReader = FgetsReader;
#endif
}

#endif /* end of include guard: CPPREAD_READER_HPP */
