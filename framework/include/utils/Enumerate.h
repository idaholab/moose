//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <iterator>
#include <iostream>

namespace Moose
{
// Forward Declarations of helper objects
template <class Iterator>
struct _enumerate_struct;

template <class Iterator>
struct _enumerate_iterator;

template <class Iterator>
struct _enumerate_range;

/**
 * Enumerate function for iterating over a range and obtaining both a reference to the underlying
 * type and an index simultaneously. This method is forward-compatible with the C++17 structured
 * bindings capability.
 *
 * C++11 compatible usage:
 *
 * for (auto it : Moose::enumerate(values))
 *   _console << it.index() << ": " << it.value() << '\n';
 *
 * // Here the third argument is the starting index value
 * for (auto it : Moose::enumerate(values.begin(), values.end(), 0))
 *   _console << it.index() << ": " << it.value() << '\n';
 *
 * C++17 usage (DO NOT USE IN MOOSE):
 *
 * for (auto [index, value] : Moose::enumerate(values))
 *   _console << index << ": " << value << '\n';
 *
 * // Here the third argument is the starting index value
 * for (auto [index, value] : Moose::enumerate(values.begin(), values.end(), 0))
 *   _console << index << ": " << value << '\n';
 */
template <class Iterator>
_enumerate_range<Iterator>
enumerate(Iterator first,
          Iterator last,
          typename std::iterator_traits<Iterator>::difference_type initial)
{
  return _enumerate_range<Iterator>(first, last, initial);
}

template <class Container>
_enumerate_range<typename Container::iterator>
enumerate(Container & content)
{
  return _enumerate_range<typename Container::iterator>(std::begin(content), std::end(content), 0);
}

template <class Container>
_enumerate_range<typename Container::const_iterator>
enumerate(const Container & content)
{
  return _enumerate_range<typename Container::const_iterator>(
      std::begin(content), std::end(content), 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Helper object
template <class Iterator>
struct _enumerate_struct
{
  using iterator = Iterator;
  using index_type = typename std::iterator_traits<iterator>::difference_type;
  using reference = typename std::iterator_traits<iterator>::reference;

  _enumerate_struct(index_type index, iterator iterator) : l_index(index), l_iter(iterator) {}

  index_type index() { return l_index; }

  reference value() { return *l_iter; }

private:
  index_type l_index;
  iterator l_iter;
};

// Helper object
template <class Iterator>
struct _enumerate_iterator
{
  using iterator = Iterator;
  using index_type = typename std::iterator_traits<iterator>::difference_type;
  using reference = typename std::iterator_traits<iterator>::reference;

  _enumerate_iterator(index_type index, iterator iterator) : index(index), iter(iterator) {}

  _enumerate_iterator & operator++()
  {
    ++index;
    ++iter;
    return *this;
  }

  bool operator!=(const _enumerate_iterator & other) const { return iter != other.iter; }

  /**
   * When MOOSE moves to C++17, we'll switch the return type of the dereference operator and the
   * corresponding calling code. This is what
   * we'll use instead.
   *
   *  #if __cplusplus > 201402L
   *    std::pair<index_type &, reference> operator*() { return {index, *iter}; }
   *  #endif
   */
  _enumerate_struct<iterator> operator*() { return _enumerate_struct<iterator>(index, iter); }

private:
  index_type index;
  iterator iter;
};

template <class Iterator>
struct _enumerate_range
{
  using index_type = typename std::iterator_traits<Iterator>::difference_type;
  using iterator = _enumerate_iterator<Iterator>;

  _enumerate_range(Iterator first, Iterator last, index_type initial)
    : first(first), last(last), initial(initial)
  {
  }

  iterator begin() const { return iterator(initial, first); }

  iterator end() const { return iterator(0, last); }

private:
  Iterator first;
  Iterator last;
  index_type initial;
};
}
