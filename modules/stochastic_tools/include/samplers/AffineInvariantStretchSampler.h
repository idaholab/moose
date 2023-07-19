//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PMCMCBase.h"

/**
 * A class for performing Affine Invariant Ensemble MCMC with stretch sampler
 */
class AffineInvariantStretchSampler : public PMCMCBase
{
public:
  static InputParameters validParams();

  AffineInvariantStretchSampler(const InputParameters & parameters);

  virtual int decisionStep() const override { return 2; }

  /**
   * Return the vector of step size for decision making
   */
  const std::vector<Real> & getAffineStepSize() const;

protected:
  virtual void proposeSamples(const unsigned int seed_value) override;

  /// The step size for the stretch sampler
  const Real _step_size;

  /// Reporter value with the previous state of all the walkers
  const std::vector<std::vector<Real>> & _previous_state;

  /// Reporter value with the previous state of all the walkers for variance
  const std::vector<Real> & _previous_state_var;

  /// Vector of affine step sizes
  std::vector<Real> _affine_step;
};
