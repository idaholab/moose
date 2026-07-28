//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SecantInversionControl.h"

registerMooseObject("OptimizationApp", SecantInversionControl);

InputParameters
SecantInversionControl::validParams()
{
  InputParameters params = InversionControlBase::validParams();
  params.addClassDescription(
      "Adjusts a parameter postprocessor each fixed-point iteration using the secant method so "
      "that a sub-app output postprocessor matches a target function of time.");

  params.addRangeCheckedParam<Real>(
      "initial_delta",
      1e-3,
      "initial_delta>0",
      "Perturbation applied to the parameter on the first iteration of each fixed-point sweep to "
      "seed the secant method.");

  return params;
}

SecantInversionControl::SecantInversionControl(const InputParameters & parameters)
  : InversionControlBase(parameters),
    _initial_delta(getParam<Real>("initial_delta")),
    _p_prev(0.0),
    _y_prev(0.0)
{
}

void
SecantInversionControl::execute()
{
  const unsigned int it = sweepIteration();
  const Real p_used = _param;
  const Real y = _output;
  const Real y_target = targetValue();

  Real p_next;
  if (it == 1)
    // Only one (p, y) pair known: seed the secant method with a perturbation.
    p_next = p_used + _initial_delta;
  else
    p_next = linearRootUpdate(_p_prev, _y_prev, p_used, y, y_target);

  // _p_prev/_y_prev are plain members, not restartable: they are re-seeded on the first iteration
  // of every sweep (it == 1), so a recover at a step boundary loses nothing.
  _p_prev = p_used;
  _y_prev = y;

  // Report the normalized convergence residual for the current (p_used, y) sample.
  setResidual(normalizedResidual(y));

  // Publish the parameter that produced the current output (the solution at convergence).
  publishConvergedParameter(p_used);

  // Write the next guess back to the working parameter postprocessor.
  setParameter(p_next);
}
