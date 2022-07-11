//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiDimPolynomialGenerator.h"

namespace StochasticTools
{

std::vector<std::vector<unsigned int>>
MultiDimPolynomialGenerator::generateTuple(const unsigned int ndim,
                                           const unsigned int order,
                                           const bool include_bias)
{
  // Compute full tensor tuple
  std::vector<std::vector<unsigned int>> tuple_1d(ndim);
  for (unsigned int d = 0; d < ndim; ++d)
  {
    tuple_1d[d].resize(order);
    for (unsigned int i = 0; i < order; ++i)
      tuple_1d[d][i] = i;
  }

  CartesianProduct<unsigned int> tensor_tuple(tuple_1d);

  // Remove polynomials that exceed the maximum order
  std::vector<std::vector<unsigned int>> tuple;
  for (unsigned int p = 0; p < tensor_tuple.numRows(); ++p)
  {
    std::vector<unsigned int> dorder = tensor_tuple.computeRow(p);
    unsigned int sum = std::accumulate(dorder.begin(), dorder.end(), 0);
    if (sum < order)
      tuple.push_back(dorder);
  }

  std::sort(tuple.begin(), tuple.end(), sortTuple);

  if (!include_bias)
    tuple.erase(tuple.begin()); // Erase intercept terms.

  return tuple;
}

bool
MultiDimPolynomialGenerator::sortTuple(const std::vector<unsigned int> & first,
                                       const std::vector<unsigned int> & second)
{
  // Sort by sum
  if (std::accumulate(first.begin(), first.end(), 0) <
      std::accumulate(second.begin(), second.end(), 0))
    return true;
  else if (std::accumulate(first.begin(), first.end(), 0) >
           std::accumulate(second.begin(), second.end(), 0))
    return false;

  // Loop over elements
  for (unsigned int d = 0; d < first.size(); ++d)
  {
    if (first[d] == second[d])
      continue;
    return (first[d] > second[d]);
  }

  return false;
}

}
