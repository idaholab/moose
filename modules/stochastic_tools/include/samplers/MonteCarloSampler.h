//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MONTECARLOSAMPLER_H
#define MONTECARLOSAMPLER_H

#include "Sampler.h"

class MonteCarloSampler;

template <>
InputParameters validParams<MonteCarloSampler>();
/**
 * A class used to perform Monte Carlo Sampling
 */
class MonteCarloSampler : public Sampler
{
public:
  MonteCarloSampler(const InputParameters & parameters);

protected:
  virtual std::vector<DenseMatrix<Real>> sample() override;

  /// Number of monte carlo samples to create for each distribution
  const std::size_t _num_samples;
};

#endif /* MONTECARLOSAMPLER_H */
