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
#include <numeric>
#include <vector>
#include <deque>
#include "libmesh/libmesh_common.h"

using namespace libMesh;

namespace StochasticTools
{
/* Compute the Cartesian Product of the supplied vectors.
 * http://phrogz.net/lazy-cartesian-product
 * https://github.com/iamtheburd/lazy-cartesian-product-python/blob/master/LazyCartesianProduct.py
 */
template <class T>
class CartesianProduct
{
public:
  CartesianProduct(const std::vector<std::vector<T>> & items);

  /// Compute the complete Cartesian product matrix
  std::vector<std::vector<T>> computeMatrix() const;

  /// Compute specified row of Cartesian product matrix
  std::vector<T> computeRow(std::size_t row) const;

  /// Compute specific value, given row and column, of the Cartesian product matrix
  T computeValue(std::size_t row, std::size_t col) const;

  /// Total number of rows in the complete matrix
  std::size_t numRows() const { return _n_rows; }

  /// Total number of columns in the complete matrix
  std::size_t numCols() const { return _n_cols; }

protected:
  /// Number of rows/columns
  const std::size_t _n_rows;
  const std::size_t _n_cols;

private:
  /// Data used to create Cartesian product
  /// use a copy because a temporary can be supplied, as is the case in the CartesianProductSampler
  const std::vector<std::vector<T>> _items;

  /// Containers for lazy Cartesian product calculation
  std::deque<unsigned int> _denomenators;
  std::deque<unsigned int> _moduli;

  /// Helper to compute the rows in initialization list to allow _n_rows to be const
  static std::size_t computeRowCount(const std::vector<std::vector<T>> & items);
};

template <typename T>
CartesianProduct<T>::CartesianProduct(const std::vector<std::vector<T>> & items)
  : _n_rows(computeRowCount(items)), _n_cols(items.size()), _items(items)
{
  dof_id_type d = 1;
  for (typename std::vector<std::vector<T>>::const_reverse_iterator iter = _items.rbegin();
       iter != _items.rend();
       ++iter)
  {
    std::size_t n = iter->size();
    _denomenators.push_front(d);
    _moduli.push_front(n);
    d *= n;
  }
}

template <typename T>
std::vector<std::vector<T>>
CartesianProduct<T>::computeMatrix() const
{
  std::vector<std::vector<T>> output(_n_rows, std::vector<T>(_n_cols));
  for (std::size_t row = 0; row < _n_rows; ++row)
    for (std::size_t col = 0; col < _n_cols; ++col)
      output[row][col] = computeValue(row, col);
  return output;
}

template <typename T>
std::vector<T>
CartesianProduct<T>::computeRow(std::size_t row) const
{
  std::vector<T> output(_n_cols);
  for (std::size_t col = 0; col < _n_cols; ++col)
    output[col] = computeValue(row, col);
  return output;
}

template <typename T>
T
CartesianProduct<T>::computeValue(std::size_t row, std::size_t col) const
{
  mooseAssert(row < _n_rows, "Row index out of range.");
  mooseAssert(col < _n_cols, "Column index out of range.");
  return _items[col][(row / _denomenators[col]) % _moduli[col]];
}

template <typename T>
std::size_t
CartesianProduct<T>::computeRowCount(const std::vector<std::vector<T>> & items)
{
  std::size_t n_rows = 1;
  for (const auto & inner : items)
    n_rows *= inner.size();
  return n_rows;
}

/*
 * Add ability to compute weighting values with the Cartesian Product
 */
template <class T, class W>
class WeightedCartesianProduct : public CartesianProduct<T>
{
public:
  WeightedCartesianProduct(const std::vector<std::vector<T>> & items,
                           const std::vector<std::vector<W>> & weights);

  /// Compute complete vector of weights
  std::vector<W> computeWeightVector() const;

  /// Compute specific weight value, given row
  W computeWeight(std::size_t row) const;

private:
  /// Data used to create Cartesian product; use a copy because a temporary can be supplied
  const CartesianProduct<W> _weight;
};

template <typename T, typename W>
WeightedCartesianProduct<T, W>::WeightedCartesianProduct(
    const std::vector<std::vector<T>> & items, const std::vector<std::vector<W>> & weights)
  : CartesianProduct<T>(items), _weight(weights)
{
  mooseAssert(items.size() == weights.size(),
              "The supplied items and weights must be the same size.");
  for (std::size_t i = 0; i < items.size(); ++i)
    mooseAssert(items[i].size() == weights[i].size(),
                "Internal vector of the supplied items and weights must be the same size.");
}

template <typename T, typename W>
std::vector<W>
WeightedCartesianProduct<T, W>::computeWeightVector() const
{
  std::vector<W> output(this->_n_rows);
  for (std::size_t i = 0; i < output.size(); ++i)
    output[i] = computeWeight(i);
  return output;
}

template <typename T, typename W>
W
WeightedCartesianProduct<T, W>::computeWeight(std::size_t row) const
{
  std::vector<W> vec = _weight.computeRow(row);
  return std::accumulate(vec.begin(), vec.end(), static_cast<W>(1), std::multiplies<W>());
}
} // namespace
