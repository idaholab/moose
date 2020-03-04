//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#pragma once
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
class CartesianProduct
{
public:
  CartesianProduct(const std::vector<std::vector<Real>> & items);

  // Compute the complete Cartesian product matrix
  std::vector<std::vector<Real>> computeMatrix() const;

  // Compute specified row of Cartesian product matrix
  std::vector<Real> computeRow(std::size_t row) const;

  // Compute specific value, given row and column, of the Cartesian product matrix
  Real computeValue(std::size_t row, std::size_t col) const;

  // Total number of rows in the complete matrix
  std::size_t numRows() const;

  // Total number of columns in the complete matrix
  std::size_t numCols() const;

protected:
  // Number of rows/columns
  const std::size_t _n_rows;
  const std::size_t _n_cols;

private:
  // Data used to create Cartesian product
  // use a copy because a temporary can be supplied, as is the case in the CartesianProductSampler
  const std::vector<std::vector<Real>> _items;

  // Containers for lazy Cartesian product calculation
  std::deque<unsigned int> _denomenators;
  std::deque<unsigned int> _moduli;

  // Helper to compute the rows in initialization list to allow _n_rows to be const
  static std::size_t computeRowCount(const std::vector<std::vector<Real>> & items);
};

/*
 * Add ability to compute weighting values with the Cartesian Product
 */
class WeightedCartesianProduct : public CartesianProduct
{
public:
  WeightedCartesianProduct(const std::vector<std::vector<Real>> & items,
                           const std::vector<std::vector<Real>> & weights);

  // Compute complete vector of weights
  std::vector<Real> computeWeightVector() const;

  // Compute specific weight value, given row
  Real computeWeight(std::size_t row) const;

private:
  // Data used to create Cartesian product; use a copy because a temporary can be supplied
  const CartesianProduct _weight;
};

} // namespace
