# cppread

Simple console input library written in C++20.

## Motivation

Using `std::cin` is just generally annoying and painful. Furthermore the error handling is quite awkward at best.

```cpp
// ...

int main() {
    // C++ standard library gives us this:
    {
        int value;                  // default initialized, uhh... not good
        std::cout << "prompt: ";
        std::cin >> value;

        if (not std::cin.good()) {                          // check all potential errors

            // handle extraction failure...

            // I might have forgot to do these two necessary steps!!
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
- Line based input: each read consume an entire line of the `stdin` (use `getline` on linux (glibc) else `getch`; define `CPPREAD_ENABLE_GETLINE` to override).
- Improved error handling: using custom type that wraps a variant: `cppread::Result<T>`.
- Exception-free: no exception thrown from `cppread::read` functions.
- No buffering: each read done from `stdin` raw (performance might be bad because of that though).
- `const` compatible: you can assign the result of `cppread::read` to a `const` variable easily.

## Example

> See [example](./example/source/main.cpp) for more examples

### Simple read

> Error handling are ignored in order to be succinct, see the next section to have a feel on how to handle errors.

```cpp
#include <cppread/read.hpp>

#include <iostream>
#include <format>

// please excuse my use of macro :)
#define println(...) std::cout << std::format(__VA_ARGS__) << '\n'

using cppread::read;

int main()
try {
    // single value read
    //-------------------
    auto result = read<int>("Please enter an integer: ");
    if (result) {
        println("value: {}", result.value());                       // retrieve contained value
    } else {
        println("cppread::Error: '{}'", toString(result.error()));  // get human readable error description (ADL in effect here)
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
#include <cppread/read.hpp>
#include <iostream>

using cppread::read, cppread::Error;

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

## Documentation

This library is a simple library (about 350 LOC, measured using `cloc`), so a dedicated documentation is not necessary. You can read the headers directly to see the documentation (Doxygen).

## Benchmark

Benchmark performed on Intel(R) Core(TM) i5-10500H (12 threads) with the frequency locked at 2.5GHz, using `hyperfine` with parameters `--warmup 3`. The benchmark involves parsing about 625k lines of 4 `(float | int)` separated by space (generated using [this script](example/random_gen.sh); `nan` removed). The benchmark code is [here](example/source/bench.cpp).

### time

|                   | `std::cin`           | `cppread` (`getline`) | `cppread` (`fgetc`)   |
| ----------------- | -------------------- | --------------------- | --------------------- |
| `615217 4-floats` | `2.280 s ±  0.012 s` | `252.6 ms ±   3.2 ms` | `449.8 ms ±   7.0 ms` |
| `625000 4-ints`   | `1.141 s ±  0.015 s` | `164.6 ms ±   1.3 ms` | `306.8 ms ±  10.0 ms` |

As you can see, this library is generally up to 9x faster than `std::cin`. Now, that is not impressive, since the fact is that `std::cin` is just that slow.

### memory

|                   | `std::cin`           | `cppread` (`getline`) | `cppread` (`fgetc`)   |
| ----------------- | -------------------- | --------------------- | --------------------- |
| `615217 4-floats` | `2.280 s ±  0.012 s` | `252.6 ms ±   3.2 ms` | `449.8 ms ±   7.0 ms` |
| `625000 4-ints`   | `1.141 s ±  0.015 s` | `164.6 ms ±   1.3 ms` | `306.8 ms ±  10.0 ms` |
