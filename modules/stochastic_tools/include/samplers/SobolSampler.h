//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
