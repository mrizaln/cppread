// #undef LINR_ENABLE_GETLINE    // uncomment this to use fgets instead of getline

#include <linr/buf_read.hpp>
#include <linr/read.hpp>

#include <boost/ut.hpp>
#include <fmt/core.h>
#include <fmt/ranges.h>

namespace ut = boost::ut;

struct Idk
{
    Idk(const Idk&)            = delete;
    Idk& operator=(const Idk&) = delete;

    Idk(Idk&&)            = default;
    Idk& operator=(Idk&&) = default;

    Idk(int i, float f)
        : m_int{ i }
        , m_float{ f }
    {
    }

    int   m_int;
    float m_float;
};

template <>
struct linr::CustomParser<Idk>
{
    Result<Idk> parse(Str str) const noexcept
    {
        auto split = linr::util::split<2>(str, ' ');
        if (not split) {
            return Error::InvalidInput;
        }

        auto i = linr::parse<int>(split->at(0)).value_or(0);
        auto f = linr::parse<float>(split->at(1)).value_or(0.0);

        return make_result<Idk>(i, f);
    }
};

void test(auto&& read)
{
    using namespace ut::literals;

    "fundamental types can be parsed"_test = [] {
        static_assert(linr::Parseable<bool>);
        static_assert(linr::Parseable<char>);
        static_assert(linr::Parseable<unsigned char>);
        static_assert(linr::Parseable<signed char>);
        static_assert(linr::Parseable<int>);
        static_assert(linr::Parseable<unsigned int>);
        static_assert(linr::Parseable<long>);
        static_assert(linr::Parseable<unsigned long>);
        static_assert(linr::Parseable<long long>);
        static_assert(linr::Parseable<unsigned long long>);
    };

    "custom type can be made Parseable"_test = [] {
        static_assert(linr::Parseable<Idk>);    //
    };

    "read a string, a whole line of it"_test = [&] {
        auto value = read.template operator()<>("insert a string: ").value();
        fmt::println("value: '{}'", value);

        // > read call above is equivalent to
        // auto value = read<std::string>(prompt, '\n');
    };

    "read a string until delimiter reached"_test = [&] {
        auto value = read.template operator()<std::string>("a string: ").value();
        fmt::println("value: '{}'", value);
    };

    "read a single char value"_test = [&] {
        auto value = read.template operator()<char>("a char: ").value();
        fmt::println("value: '{}'", value);
    };

    "multiple valued read - same type"_test = [&] {
        auto value = read.template operator()<int, int, int>("3 int: ").value();
        fmt::println("value: '{}'", value);
    };

    "multiple valued read - different types"_test = [&] {
        auto value = read.template operator()<int, double, int>("an int, a double and an int: ").value();
        fmt::println("value: '{}'", value);
    };

    // read until get value
    "read value until condition met"_test = [&] {
        int result = 0;
        while (true) {
            auto value = read.template operator()<int>("please enter an integer: ");
            if (value) {
                result = value.value();
                break;
            }

            using E = linr::Error;
            switch (value.error()) {
            case E::InvalidInput: fmt::println("Invalid input"); continue;
            case E::OutOfRange: fmt::println("Input value is out of range"); continue;
            default: break;
            }

            value = 10;
            break;
        }
        fmt::println("value: '{}'", result);
    };

    "read custom struct"_test = [&] {
        auto value = read.template operator()<Idk>("enter int and float separated by spaces: ", '\n').value();
        fmt::println("value: '{}' | '{}'", value.m_int, value.m_float);
    };

    "read getline then parse the line into custom struct"_test = [&] {
        auto str = read.template operator()<>("enter int and float separated by spaces: ").value();

        fmt::println(">>>>> {}", str);

        auto value = linr::parse<Idk>(str).value();
        fmt::println("value: '{}' | '{}'", value.m_int, value.m_float);
    };
}

int main()
{
    test([]<typename... T>(std::string_view prompt, char delim = ' ') {
        if constexpr (sizeof...(T) == 0) {
            return linr::read(prompt);
        } else {
            return linr::read<T...>(prompt, delim);
        }
    });

    test([reader = linr::BufReader(1024)]<typename... T>(std::string_view prompt, char delim = ' ') mutable {
        if constexpr (sizeof...(T) == 0) {
            return reader.read(prompt);
        } else {
            return reader.read<T...>(prompt, delim);
        }
    });
}
