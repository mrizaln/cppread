// #undef CPPREAD_ENABLE_GETLINE    // uncomment this to use fgets instead of getline

#include <cppread/read.hpp>

#include <fmt/core.h>
#include <fmt/ranges.h>
#include <boost/ut.hpp>

namespace ut = boost::ut;

struct Idk
{
    int   m_int;
    float m_float;
};

template <>
struct cppread::Parser<Idk>
{
    Result<Idk> parse(Str str) const noexcept
    {
        auto split = cppread::util::split<2>(str, ' ');
        if (not split) {
            return Error::InvalidInput;
        }

        return Idk{
            .m_int   = cppread::parse<int>(split->at(0)).value_or(0),
            .m_float = cppread::parse<float>(split->at(1)).value_or(0.0),
        };
    }
};

int main()
{

    using namespace ut::literals;

    "fundamental types can be parsed"_test = [] {
        static_assert(cppread::Parseable<bool>);
        static_assert(cppread::Parseable<char>);
        static_assert(cppread::Parseable<unsigned char>);
        static_assert(cppread::Parseable<signed char>);
        static_assert(cppread::Parseable<int>);
        static_assert(cppread::Parseable<unsigned int>);
        static_assert(cppread::Parseable<long>);
        static_assert(cppread::Parseable<unsigned long>);
        static_assert(cppread::Parseable<long long>);
        static_assert(cppread::Parseable<unsigned long long>);
    };

    "custom type can be made Parseable"_test = [] {
        static_assert(cppread::Parseable<Idk>);    //
    };

    "read a string, a whole line of it"_test = [] {
        auto value = cppread::read("insert a string: ").value();
        fmt::println("value: '{}'", value);

        // > read call above is equivalent to
        // auto value = cppread::read<std::string>(prompt, '\n');
    };

    "read a string until delimiter reached"_test = [] {
        auto value = cppread::read<std::string>("insert a string: ").value();
        fmt::println("value: '{}'", value);
    };

    "read a single char value"_test = [] {
        auto value = cppread::read<char>("insert a char: ").value();
        fmt::println("value: '{}'", value);
    };

    "multiple valued read - same type"_test = [] {
        auto value = cppread::read<int, int, int>("insert 3 integers: ").value();
        fmt::println("value: '{}'", value);
    };

    "multiple valued read - different types"_test = [] {
        auto value = cppread::read<int, double, int>("insert an integer, a double and an integer: ").value();
        fmt::println("value: '{}'", value);
    };

    // read until get value
    "read value until condition met"_test = [] {
        int result = 0;
        while (true) {
            auto value = cppread::read<int>("please enter an integer: ");
            if (value) {
                result = value.value();
                break;
            }

            using E = cppread::Error;
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

    "read custom struct"_test = [] {
        auto value = cppread::read<Idk>("enter int and float separated by spaces: ", '\n').value();
        fmt::println("value: '{}' | '{}'", value.m_int, value.m_float);
    };

    "read getline then parse the line into custom struct"_test = [] {
        auto str   = cppread::read("enter int and float separated by spaces: ").value();
        auto value = cppread::parse<Idk>(str).value();
        fmt::println("value: '{}' | '{}'", value.m_int, value.m_float);
    };
}
