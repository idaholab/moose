// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_PARSER_DETAIL_TEXT_CONFIG_HPP
#define BOOST_PARSER_DETAIL_TEXT_CONFIG_HPP

#include <boost/parser/config.hpp>

// Included for definition of __cpp_lib_concepts.
#include <iterator>


#if !BOOST_PARSER_USE_CONCEPTS
#    define BOOST_PARSER_DETAIL_TEXT_USE_CONCEPTS 0
#else
#    define BOOST_PARSER_DETAIL_TEXT_USE_CONCEPTS 1
#endif

// GCC 12 claims to support 201907L <= __cpp_deduction_guides, but does not.
#if BOOST_PARSER_DETAIL_TEXT_USE_CONCEPTS && defined(__cpp_deduction_guides) &&    \
    201907L <= __cpp_deduction_guides && (!defined(__GNUC__) || 13 <= __GNUC__) && \
    !defined(_MSC_VER)
#define BOOST_PARSER_DETAIL_TEXT_USE_ALIAS_CTAD 1
#else
#define BOOST_PARSER_DETAIL_TEXT_USE_ALIAS_CTAD 0
#endif

#if defined(__cpp_lib_ranges)
namespace boost::parser::detail { namespace text { namespace detail {
    inline constexpr auto begin = std::ranges::begin;
    inline constexpr auto end = std::ranges::end;
}}}
#else
#include <boost/parser/detail/text/detail/begin_end.hpp>
#endif

#if defined(__cpp_lib_ranges)
#    define BOOST_PARSER_DETAIL_TEXT_SUBRANGE std::ranges::subrange
#else
#    include <boost/parser/subrange.hpp>
#    define BOOST_PARSER_DETAIL_TEXT_SUBRANGE boost::parser::subrange
#endif

namespace boost::parser::detail { namespace text {
#if defined(__cpp_char8_t)
    using char8_type = char8_t;
#else
    using char8_type = char;
#endif
}}

// The inline namespaces v1 and v2 represent pre- and post-C++20.  v1 is
// inline for standards before C++20, and v2 is inline for C++20 and later.
// Note that this only applies to code for which a v2 namespace alternative
// exists.  Some instances of the v1 namespace may still be inline, if there
// is no v2 version of its contents.
#if BOOST_PARSER_DETAIL_TEXT_USE_CONCEPTS
#    define BOOST_PARSER_DETAIL_TEXT_NAMESPACE_V1 namespace v1
#    define BOOST_PARSER_DETAIL_TEXT_NAMESPACE_V2 inline namespace v2
#else
#    define BOOST_PARSER_DETAIL_TEXT_NAMESPACE_V1 inline namespace v1
#    define BOOST_PARSER_DETAIL_TEXT_NAMESPACE_V2 namespace v2
#endif

#endif
