// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_PARSER_CONFIG_HPP
#define BOOST_PARSER_CONFIG_HPP

#include <boost/parser/detail/debug_assert.hpp>

// Included for definition of __cpp_lib_concepts.
#include <iterator>


#ifdef BOOST_PARSER_DOXYGEN

/** Boost.Parser uses assertions (`BOOST_ASSERT()`) in several places to
    indicate that your use of the library has an error in it.  All of those
    places could heve instead been ill-formed code, caught at compile time.
    It is far quicker and easier to determine exactly where in your code such
    an error is located if this is a runtime failure; you can just look at the
    stack in your favorite debugger.  However, if you want to make thes kinds
    of errors always ill-formed code, define this macro. */
#    define BOOST_PARSER_NO_RUNTIME_ASSERTIONS

/** Asserts that the given condition is true.  If
    `BOOST_PARSER_NO_RUNTIME_ASSERTIONS` macro is defined by the user,
    `BOOST_PARSER_ASSERT` expends to a compile-time `static_assert()`.
    Otherwise, it expands to a run-time `BOOST_ASSERT()`. */
#    define BOOST_PARSER_ASSERT(condition)

/** Boost.Parser will automatically use concepts to constrain templates when
    building in C++20 mode, if the compiler defines `__cpp_lib_concepts`.  To
    disable the use of concepts, define this macro. */
#    define BOOST_PARSER_DISABLE_CONCEPTS

/** Define this macro to use `std::tuple` instead of `boost::hana::tuple`
    throughout Boost.Parser. */
#    define BOOST_PARSER_DISABLE_HANA_TUPLE

/** Boost.Parser automatically treats aggregate structs as if they were
    tuples.  It uses some metaprogramming to do this.  The technique used has
    a hard limit on the number of data members a struct can have.  Re-define
    this macro to change the hard limit.  Note that large values may increase
    compile times. */
#    define BOOST_PARSER_MAX_AGGREGATE_SIZE 25

/** The subrange template that is used throughout Boost.Parser.  This will be
    `boost::parser::subrange` in C++17 builds, and `std::ranges::subrange` in
    all other builds. */
#    define BOOST_PARSER_SUBRANGE

#else

#    ifdef BOOST_PARSER_NO_RUNTIME_ASSERTIONS
#        define BOOST_PARSER_ASSERT(condition) static_assert(condition)
#    elif defined(BOOST_PARSER_HAVE_BOOST_ASSERT)
#        define BOOST_PARSER_ASSERT(condition) BOOST_ASSERT(condition)
#    else
#        define BOOST_PARSER_ASSERT(condition) assert(condition)
#    endif

#endif

#if defined(__cpp_lib_constexpr_algorithms)
#    define BOOST_PARSER_ALGO_CONSTEXPR constexpr
#else
#    define BOOST_PARSER_ALGO_CONSTEXPR
#endif

#if defined(__cpp_lib_concepts) && !defined(BOOST_PARSER_DISABLE_CONCEPTS)
#    define BOOST_PARSER_USE_CONCEPTS 1
#else
#    define BOOST_PARSER_USE_CONCEPTS 0
#endif

#if defined(__cpp_lib_ranges)
#    define BOOST_PARSER_SUBRANGE std::ranges::subrange
#else
#    include <boost/parser/subrange.hpp>
#    define BOOST_PARSER_SUBRANGE boost::parser::subrange
#endif

#if defined(BOOST_PARSER_DISABLE_HANA_TUPLE)
#    define BOOST_PARSER_USE_STD_TUPLE 1
#else
#    define BOOST_PARSER_USE_STD_TUPLE 0
#endif

#if !defined(BOOST_PARSER_MAX_AGGREGATE_SIZE)
#define BOOST_PARSER_MAX_AGGREGATE_SIZE 25
#endif

// VS2019 and VS2017 need conditional constexpr in some places, even in C++17 mode.
#if !defined(_MSC_VER) || 1930 <= _MSC_VER
#define BOOST_PARSER_CONSTEXPR constexpr
#else
#define BOOST_PARSER_CONSTEXPR
#endif

#endif
