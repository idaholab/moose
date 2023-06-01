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

/**
 * Similar to CartesianProduct, this object creates a sampling scheme that produces a grid of
 * samples, where each column has its own 1D grid.
 *
 * Example:
 *   linear_space_items = '0 1 3 0 2 4'
 *   nominal_values = '1.5 2.5'
 *
 *   result = [0.0, 2.5]
 *            [1.0, 2.5]
 *            [2.0, 2.5]
 *            [1.5, 0.0]
 *            [1.5, 2.0]
 *            [1.5, 4.0]
 *            [1.5, 6.0]
 */
class Cartesian1DSampler : public Sampler
{
public:
  static InputParameters validParams();

  Cartesian1DSampler(const InputParameters & parameters);

protected:
  /// Return the sample for the given row and column
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  /// The values to use when not sampling from a column's grid
  const std::vector<Real> & _nominal_values;
  /// The range of rows in which to apply the grid for each column
  std::vector<dof_id_type> _grid_range;
  /// The values to use when sampling from a column's grid
  std::vector<std::vector<Real>> _grid_items;
};
