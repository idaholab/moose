//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

/**
 * Generates the full fixed-point inverse-solve workflow for a single scalar parameter from one
 * `SingleParameterInverseSolve` block: a forward-model TransientMultiApp, the parameter/output
 * postprocessor transfers, the working and result postprocessors, a PostprocessorConvergence, and
 * either a SecantInversionControl or a NewtonInversionControl (selected by the 'method' parameter).
 * The user only additionally needs to point the executioner at the generated convergence
 * ('multiapp_fixed_point_convergence = <block>_convergence'), which enables the Picard loop.
 */
class SingleParameterInverseSolveAction : public Action
{
public:
  static InputParameters validParams();

  SingleParameterInverseSolveAction(const InputParameters & parameters);

  virtual void act() override;
};
