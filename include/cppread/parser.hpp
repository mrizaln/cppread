#ifndef CPPREAD_PARSER_HPP
#define CPPREAD_PARSER_HPP

#include "cppread/common.hpp"
#include "cppread/util.hpp"
#include "cppread/detail/default_parser.hpp"

#include <span>

namespace cppread
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
    concept CustomParseable = requires(const CustomParser<T> p, Str str) {
        { p.parse(str) } noexcept -> std::same_as<Result<T>>;
    };

    template <typename T>
    concept DefaultParseable = requires(const detail::DefaultParser<T> p, Str str) {
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
    constexpr Results<Ts...> parseIntoTuple(std::span<Str, sizeof...(Ts)> values) noexcept
    {
        using Seq = std::index_sequence_for<Ts...>;

        // parse into tuple of Result objects
        const auto multiparse = [&]<std::size_t... Is>(std::index_sequence<Is...>) -> Tup<Result<Ts>...> {
            return { parse<Ts>(values[Is])... };
        };
        auto maybeResult = multiparse(Seq{});

        // check whether any of the values is an error
        auto error = Opt<Error>{};
        util::forEachTuple(maybeResult, [&]<std::size_t I, typename T>(T& value) {
            if (not error.has_value() and value.is_error()) {
                error = value.error();
            }
        });
        if (error.has_value()) {
            return error.value();
        }

        // if no error, flatten the tuple and return
        const auto flatten = [&]<std::size_t... Is>(std::index_sequence<Is...>) -> Tup<Ts...> {
            return { std::move(std::get<Is>(maybeResult)).value()... };
        };
        return flatten(Seq{});
    }
}

#endif /* end of include guard: CPPREAD_PARSER_HPP */
