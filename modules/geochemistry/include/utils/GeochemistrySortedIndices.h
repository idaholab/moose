//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "MooseTypes.h"

/**
 * Utility to produce sorted information
 */
namespace GeochemistrySortedIndices
{
/**
 * Produces a vector of indices corresponding to the smallest-to-biggest entries in to_sort (or
 * biggest-to-smallest if ascending=false).  Eg: to_sort = {1, 2, 5, 3, -2} then this will return
 * {4, 0, 1, 3, 2} because to_sort[4]=-2 is the least entry, to_sort[0]=1 is the next biggest,
 * etc, and to_sort[2]=5 is the greatest. Note that this doesn't actually sort the vector to_sort,
 * but instead just produces the indices
 * @param to_sort vector to consider
 * @param ascending whether to sort in ascending order
 * @return a vector of indices corresponding to the sorted version of to_sort
 */
std::vector<unsigned> sortedIndices(const std::vector<Real> & to_sort, bool ascending);
} // namespace GeochemistrySortedIndices
