//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Steady.h"

/**
 * Test executioner to show custom convergence check of Picard iterations
 */
class SteadyWithPicardCheck : public Steady
{
public:
  static InputParameters validParams();

  SteadyWithPicardCheck(const InputParameters & parameters);

  /**
   * Calls at the beginning of every Picard iterations
   */
  virtual void preSolve() override;

private:
  /// Absolute step tolerance on a designated postprocessor
  Real _pp_step_tol;

  /// The postprocessor value saved after execute
  PostprocessorValue _pp_value_old;

  /// Reference to the postprocessor value
  const PostprocessorValue & _pp_value;
};
