//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "THMControl.h"

/**
 * Control the solve based on a postprocessor value
 *
 * If the postprocessor indicates a zero, no solve it performed, otherwise the solve is performed.
 */
class THMSolvePostprocessorControl : public THMControl
{
public:
  THMSolvePostprocessorControl(const InputParameters & parameters);

  virtual void execute();

protected:
  /// The postprocessor that determines if the solve should be done or not
  const PostprocessorValue & _solve_pps;

public:
  static InputParameters validParams();
};
