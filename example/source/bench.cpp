#include <cppread/read.hpp>
#include <cppread/buf_read.hpp>

#include <CLI/CLI.hpp>
#include <fmt/core.h>
#include <fmt/ranges.h>

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

struct EmptyReader
{
    std::size_t m_count = 0;

    template <typename... Ts>
    cppread::Results<Ts...> read()
    {
        if (++m_count > 10000) {
            m_count = 0;
            return cppread::Error::EndOfFile;
        }
        return { cppread::Tup<Ts...>{ Ts{}... } };
    }
};

const std::map<std::string, Bench> TYPE_STR{
    { "int", Bench::Int },
    { "float", Bench::Float },
    { "control", Bench::Control },
};

template <typename T>
void bench(auto& reader, bool print)
{
    namespace chr = std::chrono;
    using Clock   = chr::steady_clock;
    using Value   = std::tuple<T, T, T, T>;

    auto start  = Clock::now();
    auto values = std::vector<Value>{};
    auto count  = 0uz;

    values.reserve(1'000'000);
    while (true) {
        auto result = reader.template read<T, T, T, T>();
        if (not result) {
            break;
        } else {
            values.push_back(std::move(result).value());
            if (print) {
                fmt::println("value: {}", values.back());
            }
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

    auto type    = Bench::Float;
    auto useCin  = false;
    auto bufRead = false;
    auto verbose = false;

    app.add_option("type", type, "The type to bench")
        ->required()
        ->transform(CLI::CheckedTransformer(TYPE_STR, CLI::ignore_case));
    app.add_flag("--cin", useCin, "Use cin instead");
    app.add_flag("--buf", bufRead, "Use buffered read");
    app.add_flag("--verbose", verbose, "Print output");

    if (argc <= 1) {
        std::print("{}", app.help());
        return 0;
    }

    CLI11_PARSE(app, argc, argv);

    switch (type) {
    case Bench::Int:
        if (useCin) {
            auto reader = CinReader{};
            bench<int>(reader, verbose);
        } else if (bufRead) {
            auto reader = cppread::BufReader{ 1024 };
            bench<int>(reader, verbose);
        } else {
            auto reader = DefReader{};
            bench<int>(reader, verbose);
        }
        break;
    case Bench::Float:
        if (useCin) {
            auto reader = CinReader{};
            bench<float>(reader, verbose);
        } else if (bufRead) {
            auto reader = cppread::BufReader{ 1024 };
            bench<float>(reader, verbose);
        } else {
            auto reader = DefReader{};
            bench<float>(reader, verbose);
        }
        break;
    case Bench::Control: {
        auto reader = EmptyReader{};
        bench<float>(reader, verbose);
    } break;
    }
}
