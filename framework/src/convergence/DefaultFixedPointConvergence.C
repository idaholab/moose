//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DefaultFixedPointConvergence.h"
#include "FixedPointSolve.h"
#include "Console.h"

registerMooseObject("MooseApp", DefaultFixedPointConvergence);

InputParameters
DefaultFixedPointConvergence::validParams()
{
  InputParameters params = Convergence::validParams();
  params += FixedPointSolve::fixedPointDefaultConvergenceParams();

  params.addPrivateParam<bool>("added_as_default", false);

  params.addClassDescription("Default fixed point convergence criteria.");

  return params;
}

DefaultFixedPointConvergence::DefaultFixedPointConvergence(const InputParameters & parameters)
  : Convergence(parameters),
    _added_as_default(getParam<bool>("added_as_default")),
    _accept_max_it(getSharedExecutionerParam<bool>("accept_on_max_fixed_point_iteration")),
    _fixed_point_rel_tol(getSharedExecutionerParam<Real>("fixed_point_rel_tol")),
    _fixed_point_abs_tol(getSharedExecutionerParam<Real>("fixed_point_abs_tol")),
    _custom_pp_rel_tol(getSharedExecutionerParam<Real>("custom_rel_tol")),
    _custom_pp_abs_tol(getSharedExecutionerParam<Real>("custom_abs_tol")),
    _fixed_point_custom_pp(isParamValid("custom_pp") ? &getPostprocessorValue("custom_pp")
                                                     : nullptr),
    _pp_old(0.0),
    _pp_new(std::numeric_limits<Real>::max()),
    _pp_scaling(1.0),
    _fp_solve(getMooseApp().getExecutioner()->fixedPointSolve())
{
}

void
DefaultFixedPointConvergence::initialSetup()
{
  Convergence::initialSetup();

  checkDuplicateSetSharedExecutionerParams();
}

void
DefaultFixedPointConvergence::initialize()
{
  _pp_history.str("");
}

Convergence::MooseConvergenceStatus
DefaultFixedPointConvergence::checkConvergence(unsigned int iter)
{
  TIME_SECTION(_perfid_check_convergence);

  if (_fixed_point_custom_pp)
    computeCustomConvergencePostprocessor(iter);

  if (iter + 2 > _fp_solve.minFixedPointIts())
  {
    Real max_norm = std::max(_fp_solve.fixedPointTimestepBeginNorm(iter),
                             _fp_solve.fixedPointTimestepEndNorm(iter));

    Real max_relative_drop = max_norm / _fp_solve.fixedPointInitialNorm();

    if (_fp_solve.checkFixedPointResidualNorm() && max_norm < _fixed_point_abs_tol)
    {
      _fp_solve.setFixedPointStatus(
          FixedPointSolve::MooseFixedPointConvergenceReason::CONVERGED_ABS);
      return MooseConvergenceStatus::CONVERGED;
    }
    if (_fp_solve.checkFixedPointResidualNorm() && max_relative_drop < _fixed_point_rel_tol)
    {
      _fp_solve.setFixedPointStatus(
          FixedPointSolve::MooseFixedPointConvergenceReason::CONVERGED_RELATIVE);
      return MooseConvergenceStatus::CONVERGED;
    }

    if (std::abs(_pp_new - _pp_old) < _custom_pp_abs_tol)
    {
      _fp_solve.setFixedPointStatus(
          FixedPointSolve::MooseFixedPointConvergenceReason::CONVERGED_CUSTOM);
      return MooseConvergenceStatus::CONVERGED;
    }
    if (std::abs((_pp_new - _pp_old) / _pp_scaling) < _custom_pp_rel_tol)
    {
      _fp_solve.setFixedPointStatus(
          FixedPointSolve::MooseFixedPointConvergenceReason::CONVERGED_CUSTOM);
      return MooseConvergenceStatus::CONVERGED;
    }
  }

  if (iter + 1 == _fp_solve.maxFixedPointIts())
  {
    if (_accept_max_it)
    {
      _fp_solve.setFixedPointStatus(
          FixedPointSolve::MooseFixedPointConvergenceReason::REACH_MAX_ITS);
      return MooseConvergenceStatus::CONVERGED;
    }
    else
    {
      _fp_solve.setFixedPointStatus(
          FixedPointSolve::MooseFixedPointConvergenceReason::DIVERGED_MAX_ITS);
      return MooseConvergenceStatus::DIVERGED;
    }
  }

  return MooseConvergenceStatus::ITERATING;
}

void
DefaultFixedPointConvergence::checkDuplicateSetSharedExecutionerParams() const
{
  if (_duplicate_shared_executioner_params.size() > 0 && !_added_as_default)
  {
    std::ostringstream oss;
    oss << "The following parameters were set in both this Convergence object and the "
           "executioner:\n";
    for (const auto & param : _duplicate_shared_executioner_params)
      oss << "  " << param << "\n";
    mooseError(oss.str());
  }
}

void
DefaultFixedPointConvergence::computeCustomConvergencePostprocessor(unsigned int iter)
{
  if (iter > 0 && !getParam<bool>("direct_pp_value"))
    _pp_old = _pp_new;

  if ((iter == 0 && getParam<bool>("direct_pp_value")) || !getParam<bool>("direct_pp_value"))
    _pp_scaling = *_fixed_point_custom_pp;
  _pp_new = *_fixed_point_custom_pp;

  const auto pp_name = getParam<PostprocessorName>("custom_pp");
  _pp_history << std::setw(2) << iter + 1 << " fixed point " << pp_name << " = "
              << Console::outputNorm(std::numeric_limits<Real>::max(), _pp_new, 8) << std::endl;
  _console << _pp_history.str() << std::flush;
}
