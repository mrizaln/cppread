#ifndef CPPREAD_UTIL_HPP
#define CPPREAD_UTIL_HPP

#include "cppread/common.hpp"

#include <array>
#include <utility>

#if defined(__GLIBC__) and defined(CPPREAD_ENABLE_GETLINE)
#    include <memory>
#else
#    include <string>
#endif

namespace cppread::util
{
    /**
     * @class Line
     * @brief An owning wrapper around a line of text read from stdin.
     */
    class Line
    {
#if defined(__GLIBC__) and defined(CPPREAD_ENABLE_GETLINE)
    public:
        Line(char* data, std::size_t size) noexcept
            : m_data{ data, &free }
            , m_size{ size }
        {
        }

        Str view() const noexcept { return Str{ m_data.get(), m_size }; }

    private:
        using Ptr = std::unique_ptr<char, decltype(&free)>;

        Ptr         m_data;
        std::size_t m_size;

#else
    public:
        Line(Str data) noexcept
            : m_data{ std::move(data) }
        {
        }

        Str view() const noexcept { return m_data; }

    private:
        std::string m_data;
#endif
    };

    /**
     * @brief Read a line from stdin.
     * @return An optional containing the line read, or an empty optional if EOF is reached.
     */
    inline Opt<Line> readLine() noexcept
    {
#if defined(__GLIBC__) and defined(CPPREAD_ENABLE_GETLINE)
        char*  line  = nullptr;
        size_t len   = 0;
        auto   nread = getline(&line, &len, stdin);

        if (nread == -1) {
            return {};
        } else if (line[nread - 1] == '\n') {
            // remove trailing newline
            line[nread - 1] = '\0';
        }

        return Opt<Line>{ std::in_place, line, static_cast<std::size_t>(nread) };
#else
        // get line; no buffering whatsoever so it will be slow
        auto line = std::string{};
        line.reserve(256);

        auto ch = std::fgetc(stdin);
        while (ch != '\n' and ch != EOF) {
            line.push_back(static_cast<char>(ch));
            ch = std::fgetc(stdin);
        }

        if (std::feof(stdin)) {
            return {};
        }
        return Opt<Line>{ std::in_place, std::move(line) };
#endif
    }

    /**
     * @brief Split a string into an array of strings using a delimiter.
     *
     * @param str The string to split.
     * @param delim Delimiter to split the string by.
     * @return The array of strings, or an empty optional if the string could not be split.
     */
    template <std::size_t N>
    constexpr Opt<std::array<Str, N>> split(Str str, char delim) noexcept
    {
        std::array<Str, N> res = {};
        std::size_t        i   = 0;
        std::size_t        j   = 0;

        // in case the delimiter is at the beginning of the string
        while (str[j] == delim) {
            j++;

            if (j == str.size()) {
                return std::nullopt;
            }
        }

        while (i < N and j < str.size() and str[j] != '\0') {
            std::size_t pos = str.find(delim, j);

            // in case multiple delimiters are together
            while (pos == j) {
                j++;

                if (j == str.size()) {
                    return std::nullopt;
                }

                pos = str.find(delim, j);
            }

            if (pos == Str::npos) {
                res[i++] = str.substr(j);
                break;
            }

            res[i++] = str.substr(j, pos - j);
            j        = pos + 1;
        }

        if (i != N) {
            return std::nullopt;
        }

        return res;
    }

    /**
     * @brief Iterate over a tuple element-wise.
     *
     * @param tuple The tuple to iterate over.
     * @param fn The function to apply to each element.
     */
    template <typename T, typename Fn>
    constexpr void forEachTuple(T&& tuple, Fn&& fn)
    {
        using Tup               = std::decay_t<T>;
        constexpr std::size_t N = std::tuple_size_v<std::decay_t<Tup>>;

        const auto handler = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            (fn.template operator()<Is, std::tuple_element_t<Is, Tup>>(    //
                 std::get<Is>(std::forward<T>(tuple))
             ),
             ...);
        };

        handler(std::make_index_sequence<N>());
    }
}

#endif /* end of include guard: CPPREAD_UTIL_HPP */
