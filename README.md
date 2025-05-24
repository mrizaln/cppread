# linr

Simple console input library written in C++20.

## Motivation

Using `std::cin` is just generally annoying and painful. Furthermore the error handling is quite awkward at best.

```cpp
// ...

int main() {
    // C++ standard library gives us this:
    {
        int value;                  // garbage value, uhh... not good
        std::cout << "prompt: ";
        std::cin >> value;

        if (not std::cin.good()) {                          // check all potential errors

            // handle extraction failure...

            // these are necessary steps, and I might have forgot to do these!
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            // std::cin should be usable again after two steps above...
        }

        // finally, using the value
    }

    // why can't we just do something like this instead:
    {
        auto value = read<int>("prompt: ");
        if (not value) {                                    // check all potential errors
            // handle parse/extraction failure...
        }

        // use the value
    }
}

// ...
```

This library is intended to be used in my personal projects if I ever have the need to get inputs from `stdin`, but if you feel this library fits your need, feel free to use it. These are the features this library offer:

- Simple function-based input instead of stream-based input of `std::cin`.
- Line based input: each read consume an entire line of the `stdin` (using `getline` on linux (glibc) else `fgets`; define/undef `LINR_ENABLE_GETLINE` to override).
- Improved error handling: using custom type that wraps a variant: `linr::Result<T>`.
- Exception-free: no exception thrown from `linr::read` functions.
- Buffered or non-buffered read, it's your choice.
- `const` compatible: you can assign the result of `linr::read` to a `const` variable easily.
- Built-in parser for fundamental types (using `std::from_chars`; `bool` has separate implementation) (see the implementation [here](./include/linr/detail/default_parser.hpp)).
- Allow overriding default parser via `linr::CustomParser` specialization.
- Allow extension for custom type via specialization of `linr::CustomParser`.

## Example

> See [example](./example/source/main.cpp) for more examples

### Simple read

> Error handling are ignored in order to be succinct, see the next section to have a feel on how to handle errors.

```cpp
#include <linr/read.hpp>

#include <iostream>
#include <format>

// please excuse my use of macro :)
#define println(...) std::cout << std::format(__VA_ARGS__) << '\n'

using linr::read;

int main()
try {
    // single value read
    //-------------------
    auto result = read<int>("Please enter an integer: ");
    if (result) {
        println("value: {}", result.value());                       // retrieve contained value
    } else {
        println("linr::Error: '{}'", toString(result.error()));  // get human readable error description (ADL in effect here)
    }

    // multiple values read
    //----------------------
    auto result2 = read<int, char, float>("[int char float]: ");
    auto [i, c, f] = std::move(result2).value();                    // returned value is tuple, you can use structured binding
    println("int: {} | char: {} | float: {}", i, c, f);

    // read a whole line of string
    //-----------------------------
    auto string = read("Enter anything: ").value();                 // `value()` will throw if error was returned instead
    println("anything: {}", string);

    // use custom delimiter instead of space for multiple values read
    //----------------------------------------------------------------
    auto [ii, ff, cc] = read<int, float, char>("[int/float/char]: ", '/').value();
    println("result [{}/{}/{}]", ii, ff, cc);

} catch (std::exception& e) {
    println("exception: {}", e.what());
}
```

> Try inputting these, each line for each prompt
>
> ```
> 42
> 12938 H 3.14159
> OV;9EC6V")6o7b t6r6R P78v6bp986 vt2\32487./`97ecoP786[BVTIC;O9p 7p9h6vtV8VR8O7 IBYO ROUN]"
> 100/0.001/D
> ```

### Repeated read

For example, if you want to get an `int` with value greater than `42`, you might do something like this:

```cpp
#include <linr/read.hpp>
#include <iostream>

using linr::read, linr::Error;

// wrap the read into a function/lambda so that the returned value can be const
auto readRepeat() {
    while (true) {
        auto result = read<int>("Please enter an integer greater than 42: ");
        if (not result) {
            switch (result.error()) {
            case Error::InvalidInput:
            case Error::OutOfRange:
                std::cout << "Invalid input, please try again\n";
                continue;
            case Error::EndOfFile:
            case Error::Unknown:
                std::cout << "stdin got into an unrecoverable error state!\n";
                throw std::runtime_error{ toString(result.error()).data() };
            }
        }

        if (result.value() <= 42) {
            std::cout << "Inputted value is less than or equal to 42, please try again";
            continue;
        }

        return result.value();
    }
}

int main() {
    const int value = readRepeat();

    // consume the value...
}
```

### Custom type parser

You can parse your own type by specializing `linr::CustomParser` struct. The shape of the struct must conform to `linr::CustomParseable` concept.

```cpp
#include <linr/read.hpp>
#include <linr/parser.hpp>    // linr::CustomParser, linr::CustomParseable, linr::Parseable

struct Color
{
    float m_r;
    float m_g;
    float m_b;
};

template <>
struct linr::CustomParser<Color>
{
    Result<Color> parse(Str str) const noexcept
    {
        // parse string with the shape: `Color { <r> <g> <b> }`
        //                               0     1 2   3   4   5

        // use linr's split (repeated delimiter counted as one)
        auto parts = linr::util::split<6>(str, ' ');
        if (not parts) {
            return Error::InvalidInput;
        }

        if (parts->at(0) != "Color" || parts->at(1) != "{" || parts->at(5) != "}") {
            return Error::InvalidInput;
        }

        // parse the underlying type using default parser
        auto r = linr::parse<float>(parts->at(2));
        auto g = linr::parse<float>(parts->at(3));
        auto b = linr::parse<float>(parts->at(4));

        if (not r || not g || not b) {
            return Error::InvalidInput;
        }

        return Color{ r.value(), g.value(), b.value() };
    }
};

static_assert(linr::CustomParseable<Color>);
static_assert(linr::Parseable<Color>);

int main() {
    // the delimiter set to '\n' since the Color parser reads a substring that contains space,
    // effectively read the entire line
    auto result = linr::read<Color>("input color: ", '\n');

    // use the result ...
}
```

> See this [example](./example/source/custom_type.cpp) for overriding default parser

## Documentation

This library is a simple library (about 500 LOC, measured using `cloc`), so a dedicated documentation is not necessary. You can read the headers directly to see the documentation (Doxygen format).

## Benchmark

### time

> - Benchmark performed on Intel(R) Core(TM) i5-10500H (12 threads) with the frequency locked at 2.5GHz.
> - `hyperfine` is used with parameter `--warmup 3`.
> - The benchmark involves parsing about 625k lines of 4 `(float | int)` separated by space (generated using [this script](example/random_gen.sh); `nan` removed).
> - The benchmark code is [here](example/source/bench.cpp).

|                                      | `615217 4-floats`     | `625000 4-ints`     |
| ------------------------------------ | --------------------- | ------------------- |
| `std::cin` (unsynced)                | `1.155  s ± 0.003  s` | `298.6 ms ± 2.8 ms` |
| `linr::read` (getline)            | `261.3 ms ± 1.8   ms` | `163.7 ms ± 1.7 ms` |
| `linr::read` (fgets)              | `287.0 ms ± 1.5   ms` | `183.9 ms ± 1.3 ms` |
| `linr::BufReader::read` (getline) | `253.4 ms ± 1.8   ms` | `145.7 ms ± 1.1 ms` |
| `linr::BufReader::read` (fgets)   | `270.8 ms ± 2.2   ms` | `160.7 ms ± 1.1 ms` |

As you can see, this library is generally faster than `std::cin` (unsynced) and can gain up to 4.4x speedup.

### allocation

> - Measured using `memusage`.
> - Allocations not done by the library is deducted from the total number.

|                                      | calls to malloc/new |
| ------------------------------------ | ------------------: |
| `std::cin` (unsynced)                |           `2460901` |
| `linr::read` (getline)            |            `615219` |
| `linr::read` (fgets)              |            `615219` |
| `linr::BufReader::read` (getline) |                 `7` |
| `linr::BufReader::read` (fgets)   |                 `7` |

Since `linr::BufReader` retains its buffer for its lifetime (and it grows as needed), allocation only happen few times at the start. This can help reduce memory fragmentation.
