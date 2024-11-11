#ifndef CPPREAD_READER_HPP
#define CPPREAD_READER_HPP

#include "cppread/common.hpp"

#include <concepts>
#include <memory>

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

    struct FgetcReader
    {
        struct Line
        {
            Str view() const noexcept { return m_data; }

            std::string m_data;
        };

        Opt<Line> readline() const noexcept
        {
            auto line = Line{};
            line.m_data.reserve(256);    // little optimization

            auto ch = std::fgetc(stdin);
            while (ch != '\n' and ch != EOF) {
                line.m_data.push_back(static_cast<char>(ch));
                ch = std::fgetc(stdin);
            }

            if (std::feof(stdin)) {
                return {};
            }
            return Opt<Line>{ std::move(line) };
        }
    };

    static_assert(LineReader<FgetcReader>);

#if defined(__GLIBC__) and defined(CPPREAD_ENABLE_GETLINE)
    using NoBufReader = GetlineReader;
#else
    using NoBufReader = FgetcReader;
#endif
}

#endif /* end of include guard: CPPREAD_READER_HPP */
