#undef CPPREAD_ENABLE_GETLINE
#include <cppread/read.hpp>

#include <chrono>
#include <print>
#include <string_view>

#ifdef USE_CIN
#    include <iostream>
#endif

enum class Input
{
    Int,
    Float,
};

template <typename T>
void bench()
{
    namespace chr = std::chrono;
    using Clock   = chr::steady_clock;
    using Value   = std::tuple<T, T, T, T>;

    auto start  = Clock::now();
    auto values = std::vector<Value>{};
    auto count  = 0uz;

    while (true) {
#ifdef USE_CIN
        T a, b, c, d;
        std::cin >> a >> b >> c >> d;
        if (std::cin.fail()) {
            break;
        } else {
            values.emplace_back(a, b, c, d);
            ++count;
        }
#else
        auto result = cppread::read<T, T, T, T>();
        if (not result) {
            break;
        } else {
            values.push_back(std::move(result).value());
            ++count;
        }
#endif
    }

    auto elapsed = Clock::now() - start;

    using Ms = chr::milliseconds;
    auto ms  = chr::duration_cast<Ms>(elapsed);

    std::println("Read {} lines in {}", count, ms);
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::println("Usage: {} <int|float>", argv[0]);
        return 1;
    }

    auto input = std::string_view{ argv[1] };
    if (input == "int") {
        bench<int>();
    } else if (input == "float") {
        bench<float>();
    } else {
        std::println("Usage: {} <int|float>", argv[0]);
        return 1;
    }
}
