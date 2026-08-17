//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InversionControlBase.h"

#include "Function.h"
#include "FEProblemBase.h"
#include "Executioner.h"
#include "FixedPointSolve.h"
#include "MooseApp.h"

#include <algorithm>
#include <cmath>

InputParameters
InversionControlBase::validParams()
{
  InputParameters params = Control::validParams();

  params.addRequiredParam<PostprocessorName>(
      "output_postprocessor",
      "Postprocessor holding the sub-app output to compare against the target.");
  params.addRequiredParam<PostprocessorName>(
      "parameter_postprocessor",
      "Postprocessor (e.g. a Receiver) holding the parameter value; read and updated in place.");
  params.addParam<PostprocessorName>(
      "converged_parameter_postprocessor",
      "Optional postprocessor to which the parameter value that produced the current output is "
      "written each iteration; at convergence this holds the inverse-problem solution.");
  params.addRequiredParam<FunctionName>(
      "target_function", "Function f(t) giving the target output value at each time step.");
  params.addRequiredParam<PostprocessorName>(
      "residual_postprocessor",
      "Postprocessor (e.g. a Receiver) the control writes with the normalized convergence "
      "residual; "
      "point the fixed-point Convergence object at it with a tolerance of 1.");
  params.addRangeCheckedParam<Real>("absolute_tolerance",
                                    1e-8,
                                    "absolute_tolerance>0",
                                    "Absolute tolerance on |output - target|.");
  params.addRangeCheckedParam<Real>(
      "relative_tolerance",
      1e-6,
      "relative_tolerance>0",
      "Relative tolerance on |output - target|, taken relative to |target|.");

  // Run once per fixed-point iteration, after the sub-app has solved and its output transferred.
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_BEGIN;

  return params;
}

InversionControlBase::InversionControlBase(const InputParameters & parameters)
  : Control(parameters),
    _output(getPostprocessorValue("output_postprocessor")),
    _param(getPostprocessorValue("parameter_postprocessor")),
    _param_name(getParam<PostprocessorName>("parameter_postprocessor")),
    _converged_param_name(isParamValid("converged_parameter_postprocessor")
                              ? getParam<PostprocessorName>("converged_parameter_postprocessor")
                              : PostprocessorName("")),
    _has_converged_param(isParamValid("converged_parameter_postprocessor")),
    _target_function(getFunction("target_function")),
    _residual_name(getParam<PostprocessorName>("residual_postprocessor")),
    _abs_tol(getParam<Real>("absolute_tolerance")),
    _rel_tol(getParam<Real>("relative_tolerance"))
{
}

unsigned int
InversionControlBase::sweepIteration() const
{
  Executioner * const executioner = _app.getExecutioner();
  if (!executioner || !executioner->hasFixedPointSolve())
    mooseError("requires an executioner that runs a fixed-point (MultiApp) solve; the current "
               "executioner does not provide one.");
  // numFixedPointIts() returns _fixed_point_it + 1, i.e. 1 on the first iteration of each fresh
  // fixed-point sweep (including a restep retry).
  return executioner->fixedPointSolve().numFixedPointIts();
}

Real
InversionControlBase::targetValue() const
{
  return _target_function.value(_t);
}

void
InversionControlBase::publishConvergedParameter(Real p)
{
  if (_has_converged_param)
    _fe_problem.setPostprocessorValueByName(_converged_param_name, p);
}

void
InversionControlBase::setParameter(Real p)
{
  _fe_problem.setPostprocessorValueByName(_param_name, p);
}

Real
InversionControlBase::linearRootUpdate(Real p_a, Real y_a, Real p_b, Real y_b, Real y_target)
{
  const Real dp = p_b - p_a;
  const Real dy = y_b - y_a;
  // Guard a vanishing or non-finite denominator/slope before dividing. The magnitude-scaled
  // threshold is a pragmatic "dy too small" heuristic, not a principled relative tolerance; the
  // finiteness check also traps a NaN/Inf sample from a diverging forward solve, which the "<"
  // comparisons alone would let through (every comparison with NaN is false).
  if (!std::isfinite(dp) || !std::isfinite(dy) || std::abs(dp) < 1e-15 ||
      std::abs(dy) < 1e-15 * (std::abs(y_b) + std::abs(y_a) + 1.0))
  {
    mooseWarning("The parameter update at t = ",
                 _t,
                 " has a vanishing or non-finite denominator or slope; leaving the parameter "
                 "unchanged.");
    return p_b;
  }
  return p_b - (y_b - y_target) * dp / dy;
}

Real
InversionControlBase::normalizedResidual(Real y) const
{
  const Real target = targetValue();
  return std::abs(y - target) / std::max(_abs_tol, _rel_tol * std::abs(target));
}

void
InversionControlBase::setResidual(Real value)
{
  _fe_problem.setPostprocessorValueByName(_residual_name, value);
}
