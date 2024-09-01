#include <cppread/read.hpp>

#include <fmt/core.h>
#include <fmt/ranges.h>
#include <boost/ut.hpp>

int main()
{
    // single valued read
    {
        auto value = cppread::read<int>("insert an integer: ").value();
        fmt::print("value: {}\n", value);
    }

    // multiple valued read (same type)
    {
        auto value = cppread::read<int, int, int>("insert 3 integers: ").value();
        fmt::print("value: {}\n", value);
    }

    // multiple valued read (different types)
    {
        auto value = cppread::read<int, double, int>("insert an integer, a double and an integer: ").value();
        fmt::print("value: {}\n", value);
    }
}
