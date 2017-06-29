/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef SOBOLSAMPLER_H
#define SOBOLSAMPLER_H

#include "Sampler.h"

class SobolSampler;

template <>
InputParameters validParams<SobolSampler>();
/**
 * A class used to perform Monte Carlo Sampling
 */
class SobolSampler : public Sampler
{
public:
  SobolSampler(const InputParameters & parameters);

protected:
  virtual std::vector<DenseMatrix<Real>> sample() override;
  virtual void sampleSetUp() override;
  virtual void sampleTearDown() override;

  /// Number of Monte Carlo samples to create for each Sobol matrix
  const std::size_t _num_samples;

  ///@{
  /// Sobol Monte Carlo matrices, these are sized and cleared to avoid keeping large matrices around
  DenseMatrix<Real> _a_matrix;
  DenseMatrix<Real> _b_matrix;
  ///@}
};

#endif
