# cppread

Simple console input library written in C++20.

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

This library is an attempt at improving console input in C++ by fixing the pain points explained above by giving the following features:

- Simple function-based input instead of stream-based input of `std::cin`.
- Line based input: each read consume an entire line of the `stdin`.
- Improved error handling: using custom type that wraps a variant: `cppread::Result<T>`.
- Exception-free: no exception thrown from `cppread::read` functions (`cppread::readRepeat` functions can throw though).
- No buffering: each read done from `stdin` raw.
- `const` compatible: you can assign the result of `cppread::read` to a `const` variable easily.

  > ```cpp
  > // you certainly can do this, but it lacks error handling
  > const int value = [](int i) { std::cin >> i; }({});
  >
  > // versus this (still Result<T> though, you MUST handle errors)
  > const Result<int> value = read<int>("prompt: ");
  > ```

## Example

### Simple read

> I have ignored error handling to reduce verbosity, see the next section to have a feel on how to handle errors.

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

There are certainly a lot of times when you want to get value(s) from the console input with certain conditions. For example, if you want to get an `int` with value greater than `42`, you might do something like this:

```cpp
#include <cppread/read.hpp>
#include <iostream>

using cppread::read, cppread::Error;

int main() {
    int value = 0;
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

        value = result.value();
    }

    // consume the value...
}
```

There are helper functions defined in `<cppread/read_repeat.hpp>` header to to do just exactly that:

```cpp
#include <cppread/read_repeat.hpp>
#include <iostream>

using cppread::readRepeat, cppread::Error, cppread::Repeat;

int main() {
    const int value = readRepeat<int>("Please enter an integer greater than 42: ", [] (auto& result) {
        if (not result) {
            switch (result.error()) {
            case Error::InvalidInput:
            case Error::OutOfRange:
                std::cout << "Invalid input, please try again\n";
                return Repeat::Cont;    // signal the function to redo the read
            case Error::EndOfFile:
            case Error::Unknown:
                std::cout << "stdin got into an unrecoverable error state!\n";
                throw std::runtime_error{ toString(result.error()).data() };
            }
        }

        if (result.value() <= 42) {
            std::cout << "Inputted value is less than or equal to 42, please try again\n";
            return Repeat::Cont;        // signal the function to redo the read
        }

        return Repeat::Stop;            // signal the function to stop the read
    });

    // consume the value...
}
```

> - This is what I meant when `cppread::readRepeat` can throw.
> - Furthermore, if you return a `Repeat::Stop` while the `result` contains an error, the function will throw an `std::logic_error`

Why you would want this?

- It is easier to parse in my head when the input validation is done directly in the prompt.
- The resulting value can be `const`

### Documentation

This library is a simple library (about 350 LOC), so a dedicated documentation is not necessary. You can see the headers directly to see the documentation (Doxygen).
