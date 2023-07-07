//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ParallelMarkovChainMonteCarloDecision.h"
#include "AffineInvariantDifferentialEvolutionSampler.h"

/**
 * A class for performing Affine Invariant Ensemble MCMC with differential sampler
 */
class AffineInvariantDifferentialDecision : public ParallelMarkovChainMonteCarloDecision
{
public:
  static InputParameters validParams();

  AffineInvariantDifferentialDecision(const InputParameters & parameters);

protected:
  virtual void computeTransitionVector(std::vector<Real> & tv,
                                       std::vector<Real> & evidence) override;

private:
  /// Affine differential sampler
  const AffineInvariantDifferentialEvolutionSampler * const _aides;
};
