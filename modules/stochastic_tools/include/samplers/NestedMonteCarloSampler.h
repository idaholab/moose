//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Sampler.h"

/**
 * A class used to perform nested Monte Carlo Sampling
 */
class NestedMonteCarloSampler : public Sampler
{
public:
  static InputParameters validParams();

  NestedMonteCarloSampler(const InputParameters & parameters);

protected:
  /**
   * Return the sample for the given row and column using stateless RNG indexing.
   */
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  /// Storage for distribution objects to be utilized
  std::vector<const Distribution *> _distributions;
  /// Helper for determining the target row for the given column index:
  /// target_row = std::floor(row_index / _col_mod[col_index]) * _col_mod[col_index]
  std::vector<dof_id_type> _col_mod;
};
