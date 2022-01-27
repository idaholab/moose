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
 * A class used to perform nested Monte Carlo Sampling
 */
class NestedMonteCarloSampler : public Sampler
{
public:
  static InputParameters validParams();

  NestedMonteCarloSampler(const InputParameters & parameters);

protected:
  /// Return the sample for the given row and column
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  /// Here we need to precompute rows that might not be assigned to this processor
  virtual void sampleSetUp(const SampleMode mode) override;

  /// Storage for distribution objects to be utilized
  std::vector<const Distribution *> _distributions;
  /// The loop index for distribution
  std::vector<std::size_t> _loop_index;
  /// Helper for determining if a set of columns need to be recomputed:
  /// if (row_index % _loop_mod[_loop_index[col_index]] == 0)
  std::vector<dof_id_type> _loop_mod;
  /// Storage for row data (to be used when not recomputing a column)
  std::vector<Real> _row_data;
};
