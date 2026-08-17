//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NewtonInversionControl.h"

registerMooseObject("OptimizationApp", NewtonInversionControl);

InputParameters
NewtonInversionControl::validParams()
{
  InputParameters params = InversionControlBase::validParams();
  params.addClassDescription(
      "Adjusts a parameter postprocessor to match a sub-app output to a target function using a "
      "finite-difference Newton update formed over two consecutive fixed-point iterations.");

  params.addRangeCheckedParam<Real>(
      "parameter_delta",
      1e-3,
      "parameter_delta>0",
      "Finite-difference perturbation applied on each base iteration to estimate df/dp.");
  // Invariant: this sentinel must stay far above the fixed-point Convergence tolerance (1 for the
  // generated PostprocessorConvergence) so a perturbed iteration is never seen as converged. The
  // odd/even scheme's correctness depends on that gap never closing.
  params.addRangeCheckedParam<Real>(
      "nonconverged_residual",
      1e30,
      "nonconverged_residual>0",
      "Large residual written on perturbed iterations so that convergence is only ever declared on "
      "a base iteration (where the recorded parameter is un-perturbed). Must stay well above the "
      "convergence tolerance so a perturbed iteration is never accepted.");

  return params;
}

NewtonInversionControl::NewtonInversionControl(const InputParameters & parameters)
  : InversionControlBase(parameters),
    _parameter_delta(getParam<Real>("parameter_delta")),
    _nonconverged_residual(getParam<Real>("nonconverged_residual")),
    _p_base(0.0),
    _y_base(0.0)
{
}

void
NewtonInversionControl::execute()
{
  // sweepIteration() returns 1 on the first iteration of each fresh sweep (including a restep
  // retry). Odd -> base solve at p; even -> perturbed solve at p + parameter_delta. _p_base/_y_base
  // are plain members re-seeded on every base iteration, so no restartable state is needed.
  const unsigned int it = sweepIteration();
  const Real p_used = _param;
  const Real y = _output;
  const Real y_target = targetValue();

  if (it % 2 == 1)
  {
    // Base solve: record the sample, publish it as the solution of record, report the normalized
    // residual, and set the perturbed parameter for the next (even) iteration.
    _p_base = p_used;
    _y_base = y;
    publishConvergedParameter(p_used);
    setResidual(normalizedResidual(y));
    setParameter(p_used + _parameter_delta);
  }
  else
  {
    // Perturbed solve: a single guarded linear-model step over the base and perturbed samples is
    // the finite-difference Newton update. Force a non-converged residual so that convergence is
    // only ever declared on a base iteration (where the recorded parameter is un-perturbed).
    const Real p_next = linearRootUpdate(_p_base, _y_base, p_used, y, y_target);
    setResidual(_nonconverged_residual);
    setParameter(p_next);
  }
}
