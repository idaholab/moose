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
 * A class for performing Affine Invariant Ensemble MCMC with differential sampler
 */
class AffineInvariantDES : public PMCMCBase
{
public:
  static InputParameters validParams();

  AffineInvariantDES(const InputParameters & parameters);

  virtual int decisionStep() const override { return 2; }

protected:
  virtual void proposeSamples(const unsigned int seed_value) override;

  /**
   * Compute the differential evolution from the current state
   * @param state1 A randomly selected state 1 of the sampler
   * @param state2 A randomly selected state 2 of the sampler
   * @param rnd Random number between 0 and 1
   * @param scale The scale of the input parameter
   * @param diff Differential between two randomly selected states
   */
  void computeDifferential(
      const Real & state1, const Real & state2, const Real & rnd, const Real & scale, Real & diff);

  /**
   * Tune the internal parameters
   * @param gamma An internal parameter
   * @param b An internal parameter
   * @param scale The scale of the input parameter
   */
  void tuneParams(Real & gamma, Real & b, const Real & scale);

  /// Reporter value with the previous state of all the walkers
  const std::vector<std::vector<Real>> & _previous_state;

  /// Reporter value with the previous state of all the walkers for variance
  const std::vector<Real> & _previous_state_var;

  /// Tuning options for the internal params
  const MooseEnum & _tuning_option;

  /// Scales for the parameters
  std::vector<Real> _scales;
};
