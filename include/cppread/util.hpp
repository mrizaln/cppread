#ifndef CPPREAD_UTIL_HPP
#define CPPREAD_UTIL_HPP

#include "cppread/common.hpp"

#include <array>
#include <utility>

namespace cppread::util
{
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

        while (i < N and j < str.size() and str[j] != '\0' and str[j] != '\n') {
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
