#include <cppread/read.hpp>
#include <cppread/buf_read.hpp>

#include <CLI/CLI.hpp>

#include <chrono>
#include <iostream>
#include <map>
#include <print>
#include <utility>

enum class Bench
{
    Int,
    Float,
    Control,    // empty bench, measure overhead
};

// default reader, no buf
struct DefReader
{
    template <typename... Ts>
    auto read()
    {
        return cppread::read<Ts...>();
    }
};

struct CinReader
{
    template <typename... Ts>
    cppread::Results<Ts...> read()
    {
        std::ios_base::sync_with_stdio(false);

        auto values  = std::tuple<Ts...>{};
        auto handler = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            ((std::cin >> std::get<Is>(values)), ...);
        };
        handler(std::index_sequence_for<Ts...>{});

        if (std::cin.fail()) {
            return cppread::Error::InvalidInput;
        } else {
            return { std::move(values) };
        }
    }
};

const std::map<std::string, Bench> TYPE_STR{
    { "int", Bench::Int },
    { "float", Bench::Float },
    { "control", Bench::Control },
};

template <typename T>
void bench(auto& reader)
{
    namespace chr = std::chrono;
    using Clock   = chr::steady_clock;
    using Value   = std::tuple<T, T, T, T>;

    auto start  = Clock::now();
    auto values = std::vector<Value>{};
    auto count  = 0uz;

    while (true) {
        auto result = reader.template read<T, T, T, T>();
        if (not result) {
            break;
        } else {
            values.push_back(std::move(result).value());
            ++count;
        }
    }

    auto elapsed = Clock::now() - start;

    using Ms = chr::milliseconds;
    auto ms  = chr::duration_cast<Ms>(elapsed);

    std::println("Read {} lines in {}", count, ms);
}

int main(int argc, char** argv)
{
    auto app = CLI::App{ "cppread bench" };

    Bench type    = Bench::Float;
    bool  useCin  = false;
    bool  bufRead = false;

    app.add_option("type", type, "The type to bench")
        ->required()
        ->transform(CLI::CheckedTransformer(TYPE_STR, CLI::ignore_case));
    app.add_flag("--cin", useCin, "Use cin instead");
    app.add_flag("--bufread", bufRead, "Use buffered read");

    if (argc <= 1) {
        std::print("{}", app.help());
        return 0;
    }

    CLI11_PARSE(app, argc, argv);

    switch (type) {
    case Bench::Int:
        if (useCin) {
            auto reader = CinReader{};
            bench<int>(reader);
        } else if (bufRead) {
            auto reader = cppread::BufReader{ 1024 };
            bench<int>(reader);
        } else {
            auto reader = DefReader{};
            bench<int>(reader);
        }
        break;
    case Bench::Float:
        if (useCin) {
            auto reader = CinReader{};
            bench<float>(reader);
        } else if (bufRead) {
            auto reader = cppread::BufReader{ 1024 };
            bench<float>(reader);
        } else {
            auto reader = DefReader{};
            bench<float>(reader);
        }
        break;
    case Bench::Control:
        /* do nothing */
        break;
    }
}
