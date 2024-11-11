#ifndef CPPREAD_DETAIL_READ_HPP
#define CPPREAD_DETAIL_READ_HPP

#include "cppread/common.hpp"
#include "cppread/parser.hpp"
#include "cppread/detail/line_reader.hpp"

namespace cppread::detail
{
    template <Parseable... Ts, LineReader R>
        requires(sizeof...(Ts) >= 1)
    Results<Ts...> read_impl(R& reader, Opt<Str> prompt, char delim) noexcept
    {
        // first and foremost, check whether stdin available at all
        if (std::ferror(stdin)) {
            return Error::Unknown;
        }

        if (prompt) {
            std::fwrite(prompt->data(), sizeof(Str::value_type), prompt->size(), stdout);
        }

        auto line = reader.readline();
        if (not line) {
            return Error::EndOfFile;
        }

        auto parts = util::split<sizeof...(Ts)>(line->view(), delim);
        if (parts) {
            return parseIntoTuple<Ts...>(*parts);
        }
        return Error::InvalidInput;
    }
}

#endif /* end of include guard: CPPREAD_DETAIL_READ_HPP */
