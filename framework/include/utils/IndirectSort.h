/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef INDIRECTSORT_H
#define INDIRECTSORT_H

#include <functional>
#include <vector>
#include <algorithm>

namespace Moose
{

// Indirect Sorting Functor
template <class T>
class IndexCompare
{
  T v;
public:
  IndexCompare (T v) : v(v) {}

  bool operator() (size_t lhs, size_t rhs) const
  {
    // The part after || stabalizes quicksort omit if unstable is ok
    return v[lhs] < v[rhs] || lhs < rhs && ! (v[rhs] < v[lhs]);
  }
};

template <class T>
void indirectSort (T first, T last, std::vector<size_t> &x)
{
  x.resize(std::distance(first, last));
  for (unsigned int i=0; i<x.size(); ++i)
    x[i] = i;
  std::sort(x.begin(), x.end(), IndexCompare<T>(first));
}

} // namespace Moose

#endif //INDIRECTSORT_H
