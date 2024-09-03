#ifndef CPPREAD_DETAIL_FOR_EACH_TUPLE_HPP
#define CPPREAD_DETAIL_FOR_EACH_TUPLE_HPP

#include <tuple>

namespace cppread::util
{
    template <typename T, typename Fn>
    inline constexpr auto forEachTuple(T&& tuple, Fn&& fn)
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

#endif /* end of include guard: CPPREAD_DETAIL_FOR_EACH_TUPLE_HPP */
