# cppread

Simple console input library written in C++20.

Using `std::cin` is just generally annoying and painful. Furthermore the error handling is quite awkward at best.

```cpp
// ...

int main() {
    // ...
    int value;
    std::cout << "prompt: ";
    std::cin >> value;

    // why can't we just do something like this
    auto value = read<int>("prompt: ");
}

// ...
```

This library is an attempt at improving console input in C++ by fixing the pain points explained above by giving the following features:

- Simple function-based input instead of stream-based input of `std::cin`.
- No global state: `std::cin` is not used, `stdin` is used directly instead.
- Line based input: each read consume an entire line of the `stdin`.
- Improved error handling: using custom type that wraps a variant: `cppread::Result<T>`.
- Exception-free: no exception thrown from `cppread::read` functions (`cppread::readRepeat` can throw though).

## Example

> I have ignored error handling to reduce verbosity, see the [example](./example) for a feel on how to handle errors

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
        println("value: {}", result.value());                                       // retrieve contained value
    } else {
        println("cppread::Error: '{}'", toString(result.error()));                  // get human readable error description
    }

    // multiple values read
    //----------------------
    auto result2 = read<int, char, float>("[int char float]: ");
    auto [i, c, f] = std::move(result2).value();                                    // returned value is tuple, you can use structured binding
    println("int: {} | char: {} | float: {}", i, c, f);

    // read a whole line of string
    //-----------------------------
    auto string = read("Enter anything: ").value();                                 // `value()` will throw if error was returned instead
    println("anything: {}", string);

    // use custom delimiter instead of space for multiple values read
    //----------------------------------------------------------------
    auto [ii, ff, cc] = read<int, float, char>("[int/float/char]: ", '/').value();
    println("result [{}/{}/{}]", ii, ff, cc);


} catch (std::exception& e) {
    println("exception: {}", e.what());
}
```

> try inputting these, each line each prompt
>
> ```
> 42
> 12938 H 3.14159
> OV;9EC6V")6o7b t6r6R P78v6bp986 vt2\32487./`97ecoP786[BVTIC;O9p 7p9h6vtV8VR8O7 IBYO ROUN]"
> 100/0.001/D
> ```
