//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PMCMCDecision.h"
#include "AffineInvariantStretchSampler.h"

/**
 * A class for performing Affine Invariant Ensemble MCMC with stretch sampler
 */
class AffineInvariantStretchDecision : public PMCMCDecision
{
public:
  static InputParameters validParams();

  AffineInvariantStretchDecision(const InputParameters & parameters);

protected:
  virtual void computeTransitionVector(std::vector<Real> & tv,
                                       const std::vector<Real> & evidence) override;

private:
  /// Affine stretch sampler
  const AffineInvariantStretchSampler * const _aiss;

  /// Affine step sizes
  const std::vector<Real> & _step_size;
};
