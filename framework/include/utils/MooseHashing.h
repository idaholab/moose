//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <functional>
#include <vector>
#include <utility>
#include <set>

namespace Moose
{

/// Used for hash function specialization for Attribute objects.
inline void
hash_combine(std::size_t & /*seed*/)
{
}

/// Used to combine an existing hash value with the hash of one or more other values (v and rest).
/// For example "auto h = std::hash("hello"); hash_combine(h, my_int_val, my_float_val, etc.);"
template <typename T, typename... Rest>
inline void
hash_combine(std::size_t & seed, const T & v, Rest &&... rest)
{
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  hash_combine(seed, std::forward<Rest>(rest)...);
}

/// Used for hash function specialization for Attribute objects.
template <typename T, typename... Rest>
inline void
hash_combine(std::size_t & seed, const std::vector<T> & v, Rest &&... rest)
{
  for (auto & val : v)
    hash_combine(seed, val);
  hash_combine(seed, std::forward<Rest>(rest)...);
}

/// Used for hash function specialization for Attribute objects.
template <typename T, typename... Rest>
inline void
hash_combine(std::size_t & seed, const std::set<T> & v, Rest &&... rest)
{
  for (auto & val : v)
    hash_combine(seed, val);
  hash_combine(seed, std::forward<Rest>(rest)...);
}
}

namespace std
{

template <typename S, typename T>
struct hash<std::pair<S, T>>
{
  inline size_t operator()(const std::pair<S, T> & val) const
  {
    size_t seed = 0;
    Moose::hash_combine(seed, val.first, val.second);
    return seed;
  }
};
}
