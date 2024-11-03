#ifndef CPPREAD_READ_REPEAT_HPP
#define CPPREAD_READ_REPEAT_HPP

#include "cppread/read.hpp"

// =============================================================================
//  Forward declarations
// =============================================================================

namespace cppread
{
    /**
     * @brief Read a single value from stdin
     *
     * @tparam Fn Callable must have two call overload:
     *            - `bool(*)(cppread::Tup<T...>)` ->  called on successful read and,
     *            - `cppread::Opts<T...>(*)(cppread::Error)` -> called on failed read.
     *
     *            You can use `cppread::Visit` to create the overload or use a lambda with `if constexpr`
     *            check on its body.
     *
     * @param prompt The prompt.
     * @param fn The callable object.
     * @param delim Delimiter, only `char` so you can't use unicode.
     * @return The parsed value.
     *
     * @throw <user-provided-error> If the `fn` throws, this function will throw as well.
     */
    template <Parseable... Ts, typename Fn>
        requires(sizeof...(Ts) > 1) and RepeatFn<Fn, Tup<Ts...>>
    Tup<Ts...> readRepeat(Str prompt, Fn&& fn, char delim = ' ');

    /**
     * @brief Read a single value from stdin
     *
     * @tparam Fn Callable must have two call overload:
     *            - `bool(*)(T)` ->  called on successful read and,
     *            - `cppread::Opt<T>(*)(cppread::Error)` -> called on failed read.
     *
     *            You can use `cppread::Visit` to create the overload or use a lambda with `if constexpr`
     *            check on its body.
     *
     * @param prompt The prompt.
     * @param fn The callable object.
     * @param delim Delimiter, only `char` so you can't use unicode.
     * @return The parsed value.
     *
     * @throw <user-provided-error> If the `fn` throws, this function will throw as well.
     */
    template <Parseable T, RepeatFn<T> Fn>
    T readRepeat(Str prompt, Fn&& fn, char delim = ' ');
}

// =============================================================================
//  Implementation
// =============================================================================

namespace cppread
{
    template <Parseable... Ts, typename Fn>
        requires(sizeof...(Ts) > 1) and RepeatFn<Fn, Tup<Ts...>>
    Tup<Ts...> readRepeat(Str prompt, Fn&& fn, char delim)
    {
        while (true) {
            auto result = read<Ts...>(prompt, delim);
            bool repeat = false;

            if (result) {
                repeat = not fn(result.value());
            } else {
                auto ret = fn(result.error());
                if (ret) {
                    return std::move(ret).value();
                } else {
                    repeat = true;
                }
            }

            if (not repeat) {
                return std::move(result).value();
            }
        };
    }

    template <Parseable T, RepeatFn<T> Fn>
    T readRepeat(Str prompt, Fn&& fn, char delim)
    {
        while (true) {
            auto result = read<T>(prompt, delim);
            bool repeat = false;

            if (result) {
                repeat = not fn(result.value());
            } else {
                auto ret = fn(result.error());
                if (ret) {
                    return std::move(ret).value();
                } else {
                    repeat = true;
                }
            }

            if (not repeat) {
                return std::move(result).value();
            }
        };
    }
}

#endif /* end of include guard: CPPREAD_READ_REPEAT_HPP */
