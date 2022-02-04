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
#include <algorithm>
#include <iterator> // std::iterator_traits

namespace Moose
{

// The indirect (or index) comparison functor is templated on a random
// access iterator type, and a user comparison function.  This class
// is to be constructed with a random access iterator which points to
// the beginning of the container whose values are to be indirectly
// sorted.  This class is not to be used directly by the user.
template <class RandomAccessIterator, class UserComparisonFunctor>
struct indirect_comparator
{
  // ctor
  indirect_comparator(RandomAccessIterator r, UserComparisonFunctor c)
    : _random_access_iterator(r), _user_comp(c)
  {
  }

  // comparison operator - calls the user's comparison function on
  // v[lhs] and v[rhs]
  bool operator()(size_t lhs, size_t rhs)
  {
    // Note: operator[] is defined for random access iterators!
    return _user_comp(_random_access_iterator[lhs], _random_access_iterator[rhs]);
  }

private:
  // data
  RandomAccessIterator _random_access_iterator;
  UserComparisonFunctor _user_comp;
};

// This is a common initialization function called by the indirect_sort's.
// Should not be called directly by users...
template <class RandomAccessIterator>
void
initialize_indirect_sort(RandomAccessIterator beg,
                         RandomAccessIterator end,
                         std::vector<size_t> & b)
{
  // enough storage for all the indices
  b.resize(std::distance(beg, end));

  // iota
  for (size_t i = 0; i < b.size(); ++i)
    b[i] = i;
}

// A generic indirect sort function templated on the iterator type.  Uses
// std::less<T> for the comparisons.
template <class RandomAccessIterator>
void
indirectSort(RandomAccessIterator beg, RandomAccessIterator end, std::vector<size_t> & b)
{
  // Space in b
  initialize_indirect_sort(beg, end, b);

  // Typedef for less typing.  Note: use of std::iterator_traits means this should work with
  // naked pointers too...
  typedef std::less<typename std::iterator_traits<RandomAccessIterator>::value_type>
      LessThanComparator;

  // Construct comparator object
  indirect_comparator<RandomAccessIterator, LessThanComparator> ic(beg, LessThanComparator());

  // Sort the indices, based on the data
  std::sort(b.begin(), b.end(), ic);
}

// A generic indirect sort function templated on the iterator type *and* the comparison functor
// to be used for the ordering.
template <class RandomAccessIterator, class UserComparisonFunctor>
void
indirectSort(RandomAccessIterator beg,
             RandomAccessIterator end,
             std::vector<size_t> & b,
             UserComparisonFunctor user_comp)
{
  // Space in b
  initialize_indirect_sort(beg, end, b);

  // Construct comparator object
  indirect_comparator<RandomAccessIterator, UserComparisonFunctor> ic(beg, user_comp);

  // Sort the indices, based on the data
  std::sort(b.begin(), b.end(), ic);
}

/// Uses indices created by the indirectSort function to sort the given
/// container (which must support random access, resizing, and std::swap.
template <typename T>
void
applyIndices(T & container, const std::vector<size_t> & indices)
{
  T tmp;
  tmp.resize(container.size());
  for (size_t i = 0; i < indices.size(); i++)
    tmp[i] = container[indices[i]];
  std::swap(tmp, container);
}

} // namespace Moose
