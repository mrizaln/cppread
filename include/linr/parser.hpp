#ifndef LINR_PARSER_HPP
#define LINR_PARSER_HPP

#include "linr/common.hpp"
#include "linr/detail/default_parser.hpp"
#include "linr/util.hpp"

#include <span>

namespace linr
{
    /**
     * @brief Customization point for parsing custom (user) types.
     *
     * @tparam T Type to be parsed
     *
     * User can create a parser for a type by specializing this struct. The shape of the struct should match
     * the `CustomParseable` concept.
     */
    template <typename T>
    struct CustomParser;

    template <typename T>
    concept CustomParseable = requires (const CustomParser<T> p, Str str) {
        { p.parse(str) } noexcept -> std::same_as<Result<T>>;
    };

    template <typename T>
    concept DefaultParseable = requires (const detail::DefaultParser<T> p, Str str) {
        { p.parse(str) } noexcept -> std::same_as<Result<T>>;
    };

    template <typename T>
    concept Parseable = DefaultParseable<T> or CustomParseable<T>;

    /**
     * @brief Helper function that calls the specialized `Parser` member function.
     */
    template <Parseable T>
    Result<T> parse(Str str) noexcept
    {
        if constexpr (CustomParseable<T>) {
            return CustomParser<T>{}.parse(str);
        } else {
            return detail::DefaultParser<T>{}.parse(str);
        }
    }

    /**
     * @brief Helper function that parse span of str directly into tuple
     *
     * @tparam Ts The types to parse.
     * @param values Span of string views, the extent matches the number of types.
     * @return The resulting parsed values as tuple or an error.
     */
    template <typename... Ts>
    constexpr Results<Ts...> parse_into_tuple(std::span<Str, sizeof...(Ts)> values) noexcept
    {
        using Seq = std::index_sequence_for<Ts...>;

        // parse into tuple of Result objects
        const auto multiparse = [&]<std::size_t... Is>(std::index_sequence<Is...>) -> Tup<Result<Ts>...> {
            return { parse<Ts>(values[Is])... };
        };
        auto maybe_result = multiparse(Seq{});

        // check whether any of the values is an error
        auto error = Opt<Error>{};
        util::for_each_tuple(maybe_result, [&]<std::size_t I, typename T>(T& value) {
            if (not error.has_value() and not value) {
                error = value.error();
            }
        });
        if (error.has_value()) {
            return make_error<Tup<Ts...>>(error.value());
        }

        // if no error, flatten the tuple and return
        const auto flatten = [&]<std::size_t... Is>(std::index_sequence<Is...>) -> Tup<Ts...> {
            return { std::move(std::get<Is>(maybe_result)).value()... };
        };
        return make_result<Tup<Ts...>>(flatten(Seq{}));
    }
}

#endif /* end of include guard: LINR_PARSER_HPP */
