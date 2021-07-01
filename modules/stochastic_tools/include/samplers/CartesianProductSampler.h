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
#include "CartesianProduct.h"

/**
 * Creates samples based on the Cartesian product, see CartesianProduct in utils.
 */
class CartesianProductSampler : public Sampler
{
public:
  static InputParameters validParams();

  CartesianProductSampler(const InputParameters & parameters);

protected:
  /// Return the sample for the given row and column
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  /// Helper object for computing the CartesianProcduct values
  // This is a pointer because it cannot be created until the grid vectors are assembled from
  // the input parameters.
  std::unique_ptr<const StochasticTools::CartesianProduct<Real>> _cp_ptr = nullptr;
};
