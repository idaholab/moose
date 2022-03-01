//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

/**
 * NumNonlinearIterations is a postprocessor that reports the number of nonlinear iterations
 */

class NumNonlinearIterations : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  NumNonlinearIterations(const InputParameters & parameters);

  /**
   * Initialization to be done at each timestep
   */
  virtual void timestepSetup() override;

  virtual void initialize() override {}
  virtual void execute() override {}

  /**
   * Get the numer of nonlinear iterations
   */
  virtual Real getValue() override;

protected:
  /// Pointer to the FEProblemBase
  FEProblemBase * _fe_problem;

  /// True if we should accumulate over all nonlinear solves done as part of Picard iterations in a step.
  bool _accumulate_over_step;

  /// Stores the nonlinear iteration count
  unsigned int _num_iters;

  /// Stores the last time this was executed
  Real _time;
};
