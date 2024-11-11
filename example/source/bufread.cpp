#include "cppread/buf_read.hpp"

#include <fmt/core.h>
#include <fmt/ranges.h>

template <typename... Ts>
auto readRepeat(cppread::BufReader& reader, std::string_view prompt, std::string_view fail)
{
    while (true) {
        auto result = reader.read<Ts...>(prompt);
        if (result) {
            fmt::println("{}", result.value());
            return;
        }

        if (result.error() == cppread::Error::EndOfFile) {
            fmt::println("stdin EOF reached!");
            return;
        }
        fmt::println("{} [{}]", fail, toString(result.error()));
    }
}

int main()
{
    auto reader = cppread::BufReader(10);
    readRepeat<int>(reader, "input 1 int: ", "Please input an integer");
    readRepeat<int, int>(reader, "input 2 int: ", "Please input an integer");
    readRepeat<int, int, int>(reader, "input 3 int: ", "Please input an integer");
    readRepeat<int, int, int, int>(reader, "input 4 int: ", "Please input an integer");
}
