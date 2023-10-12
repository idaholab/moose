//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <cstddef>

namespace CompileTimeDerivativesMaterialInternal
{
namespace details
{

/**
 * Check if the given index sequence is sorted ()internal function)
 */
template <std::size_t first, std::size_t second, std::size_t... tail>
constexpr bool
is_sorted()
{
  if constexpr (first <= second)
  {
    if constexpr (sizeof...(tail) == 0)
      return true;
    else
      return is_sorted<second, tail...>();
  }
  else
    return false;
}

/**
 * Compile time evaluation of the factorial of N
 */
template <std::size_t N>
constexpr std::size_t
factorial()
{
  if constexpr (N == 0)
    return 1;
  else if constexpr (N == 1)
    return 1;
  else
    return N * factorial<N - 1>();
}

/**
 * Number of distinct order N derivatives of a function with M variables
 */
template <std::size_t N, std::size_t M>
constexpr std::size_t
num_derivatives()
{
  return factorial<N + M - 1>() / (factorial<N>() * factorial<M - 1>());
}

/**
 * Merge two index sequences into one
 */
template <std::size_t... first, std::size_t... second>
constexpr auto
merge(std::index_sequence<first...>, std::index_sequence<second...>)
{
  return std::index_sequence<first..., second...>{};
}

/**
 * Increment the first number in an index sequence, but roll over into the next number if it reaches
 * Nmax. This basically increments a base-Nmax number with its digits in reverse!
 */
template <std::size_t Nmax, std::size_t first, std::size_t... tail>
constexpr auto
increment(std::index_sequence<first, tail...>)
{
  if constexpr (first + 1 == Nmax)
  {
    if constexpr (sizeof...(tail) == 0)
      return std::index_sequence<>{};
    else
      return merge(std::index_sequence<0>{}, increment<Nmax>(std::index_sequence<tail...>{}));
  }
  else
    return std::index_sequence<first + 1, tail...>{};
}

/**
 * Compute the total number of distinct derivatives for all orders N
 */
template <std::size_t M, std::size_t... N>
constexpr std::size_t
total_derivatives(std::index_sequence<N...>)
{
  return (num_derivatives<N, M>() + ...);
}

/**
 * Take an index sequence and return an index sequence of the same length with all entries replaced
 * by zeroes
 */
template <std::size_t... Ns>
constexpr auto
zeroes(std::index_sequence<Ns...>)
{
  return std::index_sequence<(void(Ns), 0)...>{};
}

} // namespace details

/**
 * Check if the given index sequence is sorted
 */
template <std::size_t first, std::size_t second, std::size_t... tail>
constexpr bool
is_sorted(std::index_sequence<first, second, tail...>)
{
  return details::is_sorted<first, second, tail...>();
}

/**
 * A sequence of size 1 is always sorted
 */
template <std::size_t first>
constexpr bool
is_sorted(std::index_sequence<first>)
{
  return true;
}

/**
 * Compute the total number of distinct derivatives for all orders 1 through N
 */
template <std::size_t N, std::size_t M>
constexpr std::size_t
total_derivatives()
{
  // we compute derivatives of orders 0 up to and including N, and subtract 1 for
  // the 0th order derivative (underived original function)
  return details::total_derivatives<M>(std::make_index_sequence<N + 1>{}) - 1;
}

// shim for C++20 std::type_identity
template <class T>
struct type_identity
{
  using type = T;
};

/**
 * Create a tuple with sizeof...(Ns) entries, containing CTArrayRefs with tags given by the Ns...
 */
template <typename T, std::size_t... Ns>
constexpr auto
make_tuple_array(std::index_sequence<Ns...>)
{
  // we cannot default construct the tuple, so we wrap it in the type_identity template
  return type_identity<std::tuple<CompileTimeDerivatives::CTArrayRef<Ns, T, unsigned int>...>>{};
}

/**
 * Create an index sequence containing N zeroes
 */
template <std::size_t N>
constexpr auto
zeroes()
{
  return details::zeroes(std::make_index_sequence<N>{});
}

/**
 * Take all derivatives of expression listed in the index sequence
 */
template <typename T, std::size_t first, std::size_t... tags>
auto
take_derivatives(const T & expression, std::index_sequence<first, tags...>)
{
  if constexpr (sizeof...(tags) == 0)
    return expression.template D<static_cast<CompileTimeDerivatives::CTTag>(first)>();
  else
    return take_derivatives(
        expression.template D<static_cast<CompileTimeDerivatives::CTTag>(first)>(),
        std::index_sequence<tags...>{});
}

} // namespace CompileTimeDerivativesMaterialInternal
