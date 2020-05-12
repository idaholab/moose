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
  const std::vector<Real> & _upper_limits_input;

  /// Lower limit to probability for each distribution
  const std::vector<Real> & _lower_limits_input;

  /// Portion of probability within each bin for each distribution
  std::vector<Real> _size_bins;

private:
  ///@{
  /// Bin sizes and limits for each distribution; these are populated during sample setup so in the
  /// future each can be controlled
  std::vector<unsigned int> _num_bins;

  /// Upper limit to probability for each distribution
  std::vector<Real> _upper_limits;

  /// Lower limit to probability for each distribution
  std::vector<Real> _lower_limits;
  ///@}
};
