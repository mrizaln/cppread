#include <cppread/read.hpp>
#include <cppread/parser.hpp>    // cppread::CustomParser and cppread::CustomParseable

#include <fmt/core.h>

struct Color
{
    float m_r;
    float m_g;
    float m_b;
};

// custom type
template <>
struct cppread::CustomParser<Color>
{
    Result<Color> parse(Str str) const noexcept
    {
        // parse string with the shape: `Color { <r> <g> <b> }`
        //                               0     1 2   3   4   5
        auto parts = cppread::util::split<6>(str, ' ');
        if (not parts) {
            return Error::InvalidInput;
        }

        if (parts->at(0) != "Color" || parts->at(1) != "{" || parts->at(5) != "}") {
            return Error::InvalidInput;
        }

        auto r = cppread::parse<float>(parts->at(2));
        auto g = cppread::parse<float>(parts->at(3));
        auto b = cppread::parse<float>(parts->at(4));

        if (not r || not g || not b) {
            return Error::InvalidInput;
        }

        return Color{ r.value(), g.value(), b.value() };
    }
};

static_assert(cppread::CustomParseable<Color>);
static_assert(cppread::Parseable<Color>);

// override default parser
template <>
struct cppread::CustomParser<int>
{
    Result<int> parse(Str str) const noexcept
    {
        if (str != "42") {
            fmt::println("not the answer to the ultimate question of life, the universe and everything!");
            std::exit(42);
        }
        return 42;
    }
};

int main()
{
    auto get_color = [] {
        while (true) {
            // the delimiter set to '\n' since the Color parser reads a substring that contains space
            auto result = cppread::read<Color>("input color: ", '\n');

            if (not result) {
                using Err = cppread::Error;
                if (auto err = result.error(); err == Err::EndOfFile or err == Err::Unknown) {
                    fmt::println("\nstdin got into unrecoverable state");
                    std::exit(1);
                } else {
                    fmt::println("invalid input");
                    continue;
                }
            }

            return result.value();
        };
    };

    auto color = get_color();
    fmt::println("color {} | {} | {}", color.m_r, color.m_g, color.m_b);

    auto answer_to_everything = cppread::read<int>("integer: ").value();
    fmt::println("the answer to everything is: {}", answer_to_everything);
}
