#ifndef LINR_UTIL_HPP
#define LINR_UTIL_HPP

#include "linr/common.hpp"

#include <algorithm>
#include <array>
#include <utility>

namespace linr::util
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

        std::size_t i = 0;
        std::size_t j = 0;

        auto find_delim_or_null = [&](std::size_t start) {
            auto iter = std::find_if(str.begin() + start, str.end(), [&](char chr) {
                return chr == delim or chr == '\0';
            });
            return iter == str.end() ? Str::npos : static_cast<std::size_t>(iter - str.begin());
        };

        while (i < N and j < str.size() and str[j] != '\0' and str[j] != '\n') {
            while (j != str.size() and str[j] == delim) {
                ++j;
            }

            auto pos = find_delim_or_null(j);

            if (pos == Str::npos) {
                res[i++] = str.substr(j);
                break;
            }

            if (str[pos] == '\0' or str[pos] == '\n') {
                res[i++] = str.substr(j, pos - j);
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

#endif /* end of include guard: LINR_UTIL_HPP */
