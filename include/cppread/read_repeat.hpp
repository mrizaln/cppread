#ifndef CPPREAD_READ_REPEAT_HPP
#define CPPREAD_READ_REPEAT_HPP

#include "cppread/read.hpp"

// =============================================================================
//  Forward declarations
// =============================================================================

namespace cppread
{
    enum class Repeat
    {
        Cont,
        Stop,
    };

    /**
     * @brief Read multiple values from stdin.
     *
     * @param prompt The prompt.
     * @param fn User-provided validation function, return `Repeat::Cont` to redo, `Repeat::Stop` to stop.
     * @param delim Delimiter, only `char` so you can't use unicode.
     *
     * @throw std::logic_error When `fn` returns `Repeat::Stop` but value contains error.
     * @throw User-provided-error If the `fn` throws, this function will throws as well.
     */
    template <Parseable... Ts, typename Fn>
        requires(
            (sizeof...(Ts) > 1)    //
            and (std::default_initializable<Ts> and ...)
            and Callable<Fn, Repeat, Results<Ts...>&>
        )
    Tuple<Ts...> readRepeat(Str prompt, Fn&& fn, char delim = ' ');

    /**
     * @brief Read a single value from stdin.
     *
     * @param prompt The prompt.
     * @param fn User-provided validation function, return `Repeat::Cont` to redo, `Repeat::Stop` to stop.
     * @param delim Delimiter, only `char` so you can't use unicode.
     *
     * @throw std::logic_error When `fn` returns `Repeat::Stop` but value contains error.
     * @throw User-provided-error If the `fn` throws, this function will throws as well.
     */
    template <Parseable T, typename Fn>
        requires std::default_initializable<T> and Callable<Fn, Repeat, Result<T>&>
    T readRepeat(Str prompt, Fn&& fn, char delim = ' ');
}

// =============================================================================
//  Implementation
// =============================================================================

#include <stdexcept>

namespace cppread
{
    template <Parseable... Ts, typename Fn>
        requires(
            (sizeof...(Ts) > 1)    //
            and (std::default_initializable<Ts> and ...)
            and Callable<Fn, Repeat, Results<Ts...>&>
        )
    Tuple<Ts...> readRepeat(Str prompt, Fn&& fn, char delim)
    {
        while (true) {
            auto result = read<Ts...>(prompt, delim);
            auto repeat = fn(result);

            switch (repeat) {
            case Repeat::Cont: continue;
            case Repeat::Stop: {
                if (result) {
                    return std::move(result).value();
                } else {
                    throw std::logic_error{ "Parsed value contains error" };
                }
            }
            }
        };
    }

    template <Parseable T, typename Fn>
        requires std::default_initializable<T> and Callable<Fn, Repeat, Result<T>&>
    T readRepeat(Str prompt, Fn&& fn, char delim)
    {
        while (true) {
            auto result = read<T>(prompt, delim);
            auto repeat = fn(result);

            switch (repeat) {
            case Repeat::Cont: continue;
            case Repeat::Stop: {
                if (result) {
                    return std::move(result).value();
                } else {
                    throw std::logic_error{ "Parsed value contains error" };
                }
            }
            }
        };
    }
}

#endif /* end of include guard: CPPREAD_READ_REPEAT_HPP */
