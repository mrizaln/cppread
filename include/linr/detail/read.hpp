#ifndef LINR_DETAIL_READ_HPP
#define LINR_DETAIL_READ_HPP

#include "linr/common.hpp"
#include "linr/parser.hpp"
#include "linr/detail/line_reader.hpp"

namespace linr::detail
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

        auto parts = ::linr::util::split<sizeof...(Ts)>(line->view(), delim);
        if (parts) {
            return parseIntoTuple<Ts...>(*parts);
        }
        return Error::InvalidInput;
    }
}

#endif /* end of include guard: LINR_DETAIL_READ_HPP */
