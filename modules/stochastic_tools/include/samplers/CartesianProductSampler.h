//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Sampler.h"

class CartesianProductSampler;
template <>
InputParameters validParams<CartesianProductSampler>();

/**
 * Creates samples based on the Cartesian product
 * http://phrogz.net/lazy-cartesian-product
 * https://github.com/iamtheburd/lazy-cartesian-product-python/blob/master/LazyCartesianProduct.py
 */
class CartesianProductSampler : public Sampler
{
public:
  static InputParameters validParams();

  CartesianProductSampler(const InputParameters & parameters);

protected:
  /// Return the sample for the given row and column
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  /// Setup the necessary data for computing the lazy Cartesian product
  virtual void sampleSetUp() override;

  /// Data used to create Cartesian product
  std::vector<std::vector<Real>> _grid_items;

  // Containers for lazy Cartesian product calculation
  std::deque<unsigned int> _denomenators;
  std::deque<unsigned int> _moduli;
};
