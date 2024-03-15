// Copyright (C) 2023 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_PARSER_DETAIL_TEXT_TRANSCODE_ITERATOR_FWD_HPP
#define BOOST_PARSER_DETAIL_TEXT_TRANSCODE_ITERATOR_FWD_HPP

#include <boost/parser/detail/text/concepts.hpp>


namespace boost::parser::detail { namespace text {

    struct use_replacement_character;

    namespace detail {
        template<
            typename RepackedIterator,
            typename I,
            typename S,
            typename Then>
        struct bidi_repacker;
    }
}}

namespace boost::parser::detail { namespace text {

    namespace detail {
        template<format Format>
        constexpr auto format_to_type();

        template<format Format>
        using format_to_type_t = decltype(format_to_type<Format>());
    }

#if BOOST_PARSER_DETAIL_TEXT_USE_CONCEPTS
    template<
        format FromFormat,
        format ToFormat,
        std::input_iterator I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
        requires std::convertible_to<std::iter_value_t<I>, detail::format_to_type_t<FromFormat>>
#else
    template<
        format FromFormat,
        format ToFormat,
        typename I,
        typename S = I,
        typename ErrorHandler = use_replacement_character>
#endif
    class utf_iterator;

#if BOOST_PARSER_DETAIL_TEXT_USE_ALIAS_CTAD

    template<
        utf8_iter I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
    using utf_8_to_16_iterator =
        utf_iterator<format::utf8, format::utf16, I, S, ErrorHandler>;
    template<
        utf16_iter I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
    using utf_16_to_8_iterator =
        utf_iterator<format::utf16, format::utf8, I, S, ErrorHandler>;


    template<
        utf8_iter I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
    using utf_8_to_32_iterator =
        utf_iterator<format::utf8, format::utf32, I, S, ErrorHandler>;
    template<
        utf32_iter I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
    using utf_32_to_8_iterator =
        utf_iterator<format::utf32, format::utf8, I, S, ErrorHandler>;


    template<
        utf16_iter I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
    using utf_16_to_32_iterator =
        utf_iterator<format::utf16, format::utf32, I, S, ErrorHandler>;
    template<
        utf32_iter I,
        std::sentinel_for<I> S = I,
        transcoding_error_handler ErrorHandler = use_replacement_character>
    using utf_32_to_16_iterator =
        utf_iterator<format::utf32, format::utf16, I, S, ErrorHandler>;

#endif

}}

#endif
