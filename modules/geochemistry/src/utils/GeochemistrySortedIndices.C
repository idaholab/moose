//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemistrySortedIndices.h"

namespace GeochemistrySortedIndices
{
std::vector<unsigned>
sortedIndices(const std::vector<Real> & to_sort, bool ascending)
{
  std::vector<unsigned> order(to_sort.size());
  unsigned ind = 0;
  std::iota(order.begin(), order.end(), ind++);
  if (ascending)
    std::sort(order.begin(), order.end(), [&](int i, int j) { return to_sort[i] < to_sort[j]; });
  else
    std::sort(order.begin(), order.end(), [&](int i, int j) { return to_sort[i] > to_sort[j]; });
  return order;
}
} // namespace GeochemistrySortedIndices
