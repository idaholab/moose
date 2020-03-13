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

class LatinHypercubeSampler;

template <>
InputParameters validParams<LatinHypercubeSampler>();
/**
 * A class used to perform Monte Carlo Sampling
 */
class LatinHypercubeSampler : public Sampler
{
public:
  static InputParameters validParams();

  LatinHypercubeSampler(const InputParameters & parameters);

protected:
  virtual void sampleSetUp() override;
  virtual void sampleTearDown() override;
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  /// Storage for distribution objects to be utilized
  std::vector<Distribution const *> _distributions;

  /// Distribution names
  const std::vector<DistributionName> & _distribution_names;

  /// Number of intervals for each distribution
  const std::vector<unsigned int> & _num_bins_input;

  /// Upper limit to probability for each distribution
  const std::vector<Real> & _upper_limits;

  /// Lower limit to probability for each distribution
  const std::vector<Real> & _lower_limits;

  /// Portion of probability within each bin for each distribution
  std::vector<Real> _size_bins;

  /// Bin sizes for each distribution; this is populated during sample setup so in the future
  /// it can be controlled
  std::vector<unsigned int> _num_bins;
};
