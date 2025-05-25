#ifndef LINR_DETAIL_READ_HPP
#define LINR_DETAIL_READ_HPP

#include "linr/common.hpp"
#include "linr/detail/line_reader.hpp"
#include "linr/parser.hpp"

namespace linr::detail
{
    template <Parseable... Ts, LineReader R>
        requires (sizeof...(Ts) >= 1) and (std::movable<Ts> and ...)
    Results<Ts...> read_impl(R& reader, Opt<Str> prompt, char delim) noexcept
    {
        // first and foremost, check whether stdin available at all
        if (std::ferror(stdin)) {
            return make_error<Tup<Ts...>>(Error::Unknown);
        }

        if (prompt) {
            std::fwrite(prompt->data(), sizeof(Str::value_type), prompt->size(), stdout);
        }

        auto line = reader.readline();
        if (not line) {
            return make_error<Tup<Ts...>>(Error::EndOfFile);
        }

        auto parts = util::split<sizeof...(Ts)>(line->view(), delim);
        if (parts) {
            return parse_into_tuple<Ts...>(*parts);
        }
        return make_error<Tup<Ts...>>(Error::InvalidInput);
    }
}

#endif /* end of include guard: LINR_DETAIL_READ_HPP */
