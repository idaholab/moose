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
 * A class for performing Affine Invariant Ensemble MCMC with stretch sampler
 */
class AffineInvariantStretchSampler : public ParallelMarkovChainMonteCarloBase
{
public:
  static InputParameters validParams();

  AffineInvariantStretchSampler(const InputParameters & parameters);

protected:
  virtual void sampleSetUp(const Sampler::SampleMode mode) override;

private:
  /// The step size for the stretch sampler
  const Real & _step_size;

  /// Reporter value with the previous state of all the walkers
  const std::vector<std::vector<Real>> & _previous_state;
};
