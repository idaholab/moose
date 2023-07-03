//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ParallelMarkovChainMonteCarloBase.h"

/**
 * A class for performing M-H MCMC sampling with independent Gaussian propoposals
 */
class IndependentGaussianMetropolisHastings : public ParallelMarkovChainMonteCarloBase
{
public:
  static InputParameters validParams();

  IndependentGaussianMetropolisHastings(const InputParameters & parameters);

protected:
  virtual void sampleSetUp(const Sampler::SampleMode mode) override;

private:
  /// Reporter value the seed input values for proposing the next set of samples
  const std::vector<Real> & _seed_inputs;

  /// Standard deviations for making the next proposal
  const std::vector<Real> & _std_prop;

  /// Initial values of the input params to get the MCMC scheme started
  const std::vector<Real> & _initial_values;
};
