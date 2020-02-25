//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "CartesianProduct.h"

namespace StochasticTools
{

CartesianProduct::CartesianProduct(const std::vector<std::vector<Real>> & items)
  : _n_rows(computeRowCount(items)), _n_cols(items.size()), _items(items)
{
  dof_id_type d = 1;
  for (std::vector<std::vector<Real>>::const_reverse_iterator iter = _items.rbegin();
       iter != _items.rend();
       ++iter)
  {
    std::size_t n = iter->size();
    _denomenators.push_front(d);
    _moduli.push_front(n);
    d *= n;
  }
}

std::size_t
CartesianProduct::numRows() const
{
  return _n_rows;
}

std::size_t
CartesianProduct::numCols() const
{
  return _n_cols;
}

std::vector<std::vector<Real>>
CartesianProduct::computeMatrix() const
{
  std::vector<std::vector<Real>> output(_n_rows, std::vector<Real>(_n_cols));
  for (std::size_t row = 0; row < _n_rows; ++row)
    for (std::size_t col = 0; col < _n_cols; ++col)
      output[row][col] = computeValue(row, col);
  return output;
}

std::vector<Real>
CartesianProduct::computeRow(std::size_t row) const
{
  std::vector<Real> output(_n_cols);
  for (std::size_t col = 0; col < _n_cols; ++col)
    output[col] = computeValue(row, col);
  return output;
}

Real
CartesianProduct::computeValue(std::size_t row, std::size_t col) const
{
  mooseAssert(row < _n_rows, "Row index out of range.");
  mooseAssert(col < _n_cols, "Column index out of range.");
  return _items[col][(row / _denomenators[col]) % _moduli[col]];
}

std::size_t
CartesianProduct::computeRowCount(const std::vector<std::vector<Real>> & items)
{
  std::size_t n_rows = 1;
  for (const auto & inner : items)
    n_rows *= inner.size();
  return n_rows;
}

WeightedCartesianProduct::WeightedCartesianProduct(const std::vector<std::vector<Real>> & items,
                                                   const std::vector<std::vector<Real>> & weights)
  : CartesianProduct(items), _weight(weights)
{
  mooseAssert(items.size() == weights.size(),
              "The supplied items and weights must be the same size.");
  for (std::size_t i = 0; i < items.size(); ++i)
    mooseAssert(items[i].size() == weights[i].size(),
                "Internal vector of the supplied items and weights must be the same size.");
}

std::vector<Real>
WeightedCartesianProduct::computeWeightVector() const
{
  std::vector<Real> output(_n_rows);
  for (std::size_t i = 0; i < output.size(); ++i)
    output[i] = computeWeight(i);
  return output;
}

Real
WeightedCartesianProduct::computeWeight(std::size_t row) const
{
  std::vector<Real> vec = _weight.computeRow(row);
  return std::accumulate(vec.begin(), vec.end(), 1, std::multiplies<Real>());
}

} // namespace
