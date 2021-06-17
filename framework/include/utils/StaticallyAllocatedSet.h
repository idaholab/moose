//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseError.h"

#include <array>

namespace MooseUtils
{

/**
 * Optimized set with static allocation.
 *
 * Useful when you a set that is small and doesn't do any allocation.
 * Small in this context is the size at which you are not concerned with
 * the linear search that involves a comparison of T.
 *
 * It is templated on the type the set will hold and the maximum size of the set (N)
 */
template <typename T, std::size_t N>
class StaticallyAllocatedSet
{
public:
  typedef typename std::array<T, N>::iterator iterator;
  typedef typename std::array<T, N>::const_iterator const_iterator;

  /**
   * Create a set
   */
  StaticallyAllocatedSet() : _end_pos(0) {}

  /**
   * Number of entries
   */
  std::size_t size() const { return _end_pos; }

  /**
   * Whether or not the set is empty
   */
  bool empty() const { return !_end_pos; }

  /**
   * Add a new entry to the set
   */
  void insert(const T & value)
  {
    if (contains(value))
      return;

    if (_end_pos == N)
      mooseError("Out of space in StaticallyAllocatedSet (size = ", N, ")");

    _data[_end_pos] = value;
    ++_end_pos;
  }

  /**
   * Remove all entries
   *
   * Note: this does NOT at all free any entries
   */
  void clear() { _end_pos = 0; }

  /**
   * Iterator for the first entry
   */
  iterator begin() { return _data.begin(); }
  /**
   * Const iterator for the first entry
   */
  const_iterator begin() const { return _data.begin(); }

  /**
   * Iterator for the last entry
   */
  iterator end() { return _data.begin() + _end_pos; }
  /**
   * Const iterator for the last entry
   */
  const_iterator end() const { return _data.begin() + _end_pos; }

  /**
   * Whether or not the set contains the given item
   */
  bool contains(const T & value) const
  {
    for (std::size_t i = 0; i < _end_pos; i++)
      if (_data[i] == value)
        return true;

    return false;
  }

  /**
   * Swap the contents of this set with another
   */
  void swap(StaticallyAllocatedSet<T, N> & other)
  {
    _data.swap(other._data);
    std::swap(_end_pos, other._end_pos);
  }

  /**
   * Expert interface: the current ending position
   */
  std::size_t dataEndPos() const { return _end_pos; }

private:
  /// The data
  std::array<T, N> _data;

  /// Save the ending as positions internally
  std::size_t _end_pos;
};

} // namespace MooseUtils
