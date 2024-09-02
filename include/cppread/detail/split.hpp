#ifndef CPPREAD_DETAIL_SPLIT_HPP
#define CPPREAD_DETAIL_SPLIT_HPP

#include <array>
#include <optional>
#include <string_view>

namespace cppread::detail
{
    template <std::size_t N>
    using Split = std::array<std::string_view, N>;

    template <std::size_t N>
    constexpr std::optional<Split<N>> split(std::string_view str, char delim) noexcept
    {
        Split<N>    result = {};
        std::size_t i      = 0;
        std::size_t j      = 0;

        // in case the delimiter is at the beginning of the string
        while (str[j] == delim) {
            j++;

            if (j == str.size()) {
                return std::nullopt;
            }
        }

        while (i < N and j < str.size()) {
            std::size_t pos = str.find(delim, j);

            // in case multiple delimiters are together
            while (pos == j) {
                j++;

                if (j == str.size()) {
                    return std::nullopt;
                }

                pos = str.find(delim, j);
            }

            if (pos == std::string_view::npos) {
                result[i++] = str.substr(j);
                break;
            }

            result[i++] = str.substr(j, pos - j);
            j           = pos + 1;
        }

        if (i != N) {
            return std::nullopt;
        }

        return result;
    }
}

#endif /* end of include guard: CPPREAD_DETAIL_SPLIT_HPP */