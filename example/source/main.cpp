#include <cppread/read.hpp>
#include <cppread/buf_read.hpp>

#include <iostream>
#include <format>

// please excuse my use of macro :)

#define println(...) std::cout << std::format(__VA_ARGS__) << '\n'

using cppread::read;

int main()
try {
    // single value read
    {
        // read is exception-free, but it returns a cppread::Result<T> instead
        auto result = read<int>("Please enter an integer: ");

        // can be tested for error using bool conversion
        if (not result) {
            using E = cppread::Error;

            switch (result.error()) {
                // parse error
            case E::InvalidInput:    // what to do when input is invalid
            case E::OutOfRange:      // parsed value can't be contained within given type (integer)

                // stdin error
            case E::EndOfFile:    // stdin EOF reached, usually you can't recover from this
            case E::Unknown:      // Unknown error happen to stdin, platform-specific, can't recover
                                  // errno might help

                // i'll just throw for now
            default: throw result.error();
            }
        }

        println("value: {}", result.value());
    }

    // multiple values read
    {
        auto result = read<int, char, std::string>("Please enter an [int, char, and std::string]: ");
        if (not result) {
            // ... what to do on error ...

            // for example, throw
            throw result.error();
        }

        // returned value is tuple, you can use structured binding
        auto [vInt, vChar, vString] = std::move(result).value();

        println("int: {} | char: {} | string: {}", vInt, vChar, vString);
    }

    // read a whole line of string
    {
        auto string = read("Enter anything: ").value();    // `value()` will throw if error was returned
        println("anything: {}", string);
    }

    // read repeatedly until condition met (using if constexpr lambda)
    {
        const auto readRepeat = [] {
            while (true) {
                auto value = read<int>("integer greater than 10: ");
                if (value and value.value() > 10) {
                    return value.value();
                } else {
                    switch (value.error()) {
                    case cppread::Error::EndOfFile: [[fallthrough]];
                    case cppread::Error::Unknown: return 100;
                    default: continue;
                    }
                }
            }
        };
        const auto value = readRepeat();

        println("value: {}", value);
    }

    // read repeatedly until condition met (using cppread::Visit)
    {
        const auto readRepeat = [] {
            while (true) {
                auto result = read<int, int>("two integer (first one must be greater than 10): ");
                if (result) {
                    const auto& [left, _] = result.value();
                    if (left > 10) {
                        return result.value();
                    }
                } else {
                    switch (result.error()) {
                    case cppread::Error::EndOfFile: [[fallthrough]];
                    case cppread::Error::Unknown: return std::tuple{ 100, 100 };
                    default: continue;
                    }
                }
            }
        };
        const auto [value1, value2] = readRepeat();

        println("value1: {} | value2: {}", value1, value2);
    }

    {
        int value = 0;
        while (value <= 10) {
            auto result = read<int>("enter an integer greater than 10: ");
            if (not result) {
                switch (result.error()) {
                case cppread::Error::EndOfFile: [[fallthrough]];
                case cppread::Error::Unknown: value = 100;
                default: /* do nothing */;
                }
            } else {
                value = result.value();
            }
        }
    }

} catch (cppread::Error error) {

    println("cppread::Error: '{}'", cppread::toString(error));
}
