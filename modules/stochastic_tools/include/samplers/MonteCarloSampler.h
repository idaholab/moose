/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
