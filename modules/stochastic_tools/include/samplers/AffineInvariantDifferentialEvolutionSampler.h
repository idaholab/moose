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
class AffineInvariantDifferentialEvolutionSampler : public ParallelMarkovChainMonteCarloBase
{
public:
  static InputParameters validParams();

  AffineInvariantDifferentialEvolutionSampler(const InputParameters & parameters);

protected:
  virtual void sampleSetUp(const Sampler::SampleMode mode) override;

private:
  /// Compute the differential evolution from the current state
  void computeDifferential(const Real & state1, const Real & state2, const unsigned int & seed, Real & diff);

  /// Tune the internal parameters
  void tuneParams(Real & gamma, Real & b);

  /// Reporter value with the previous state of all the walkers
  const std::vector<std::vector<Real>> & _previous_state;

  /// Tuning options for the internal params
  const MooseEnum & _tuning_option;
};
