//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DefaultMultiAppFixedPointConvergence.h"
#include "FEProblemBase.h"
#include "FixedPointSolve.h"
#include "Console.h"

registerMooseObject("MooseApp", DefaultMultiAppFixedPointConvergence);

InputParameters
DefaultMultiAppFixedPointConvergence::validParams()
{
  InputParameters params = DefaultConvergenceBase::validParams();
  params += FixedPointSolve::fixedPointDefaultConvergenceParams();

  params.addClassDescription("Default fixed point convergence criteria.");

  return params;
}

DefaultMultiAppFixedPointConvergence::DefaultMultiAppFixedPointConvergence(
    const InputParameters & parameters)
  : DefaultConvergenceBase(parameters),
    _has_fixed_point_norm(
        !getSharedExecutionerParam<bool>("disable_fixed_point_residual_norm_check")),
    _fixed_point_force_norms(getSharedExecutionerParam<bool>("fixed_point_force_norms")),
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
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _fp_solve(getMooseApp().getExecutioner()->fixedPointSolve())
{
  if (!_has_fixed_point_norm && parameters.isParamSetByUser("fixed_point_rel_tol"))
    paramWarning(
        "disable_fixed_point_residual_norm_check",
        "fixed_point_rel_tol will be ignored because the fixed point residual check is disabled.");
  if (!_has_fixed_point_norm && parameters.isParamSetByUser("fixed_point_abs_tol"))
    paramWarning(
        "disable_fixed_point_residual_norm_check",
        "fixed_point_abs_tol will be ignored because the fixed point residual check is disabled.");
  if (!_has_fixed_point_norm && parameters.isParamSetByUser("fixed_point_force_norms"))
    paramWarning("disable_fixed_point_residual_norm_check",
                 "fixed_point_force_norms will be ignored because the fixed point residual check "
                 "is disabled.");
}

void
DefaultMultiAppFixedPointConvergence::initialize()
{
  DefaultConvergenceBase::initialize();

  _fixed_point_timestep_begin_norm.clear();
  _fixed_point_timestep_end_norm.clear();
  _fixed_point_timestep_begin_norm.resize(_fp_solve.maxFixedPointIts());
  _fixed_point_timestep_end_norm.resize(_fp_solve.maxFixedPointIts());

  _pp_history.str("");

  // compute residual norm before iteration loop
  if (_has_fixed_point_norm)
  {
    _fixed_point_initial_norm = _fe_problem.computeResidualL2Norm();
    _console << COLOR_MAGENTA << "Initial fixed point residual norm: " << COLOR_DEFAULT;
    if (_fixed_point_initial_norm == std::numeric_limits<Real>::max())
      _console << " MAX ";
    else
      _console << std::scientific << _fixed_point_initial_norm;
    _console << COLOR_DEFAULT << "\n" << std::endl;
  }
}

void
DefaultMultiAppFixedPointConvergence::preExecute()
{
  DefaultConvergenceBase::preExecute();

  // compute TIMESTEP_BEGIN residual norm
  if (_has_fixed_point_norm &&
      (_fe_problem.hasMultiApps(EXEC_TIMESTEP_BEGIN) || _fixed_point_force_norms))
  {
    const auto iter = _fp_solve.numFixedPointIts() - 1;
    _fixed_point_timestep_begin_norm[iter] = _fe_problem.computeResidualL2Norm();

    Real begin_norm_old =
        (iter > 0 ? _fixed_point_timestep_begin_norm[iter - 1] : std::numeric_limits<Real>::max());

    _console << COLOR_MAGENTA << "Fixed point residual norm after TIMESTEP_BEGIN MultiApps: "
             << Console::outputNorm(begin_norm_old, _fixed_point_timestep_begin_norm[iter])
             << std::endl;
  }
}

Convergence::MooseConvergenceStatus
DefaultMultiAppFixedPointConvergence::checkConvergence(unsigned int iter)
{
  TIME_SECTION(_perfid_check_convergence);

  if (_fixed_point_custom_pp)
    computeCustomConvergencePostprocessor(iter);

  // compute TIMESTEP_END residual norm
  if (_has_fixed_point_norm)
    if (_fe_problem.hasMultiApps(EXEC_TIMESTEP_END) || _fixed_point_force_norms)
    {
      _fixed_point_timestep_end_norm[iter] = _fe_problem.computeResidualL2Norm();

      Real end_norm_old =
          (iter > 0 ? _fixed_point_timestep_end_norm[iter - 1] : std::numeric_limits<Real>::max());

      _console << COLOR_MAGENTA << "Fixed point residual norm after TIMESTEP_END MultiApps: "
               << Console::outputNorm(end_norm_old, _fixed_point_timestep_end_norm[iter])
               << std::endl;
    }

  // print residual norm history
  if (_has_fixed_point_norm)
    _fp_solve.printFixedPointConvergenceHistory(_fixed_point_initial_norm,
                                                _fixed_point_timestep_begin_norm,
                                                _fixed_point_timestep_end_norm);

  if (iter + 2 > _fp_solve.minFixedPointIts())
  {
    Real max_norm =
        std::max(_fixed_point_timestep_begin_norm[iter], _fixed_point_timestep_end_norm[iter]);

    Real max_relative_drop = max_norm / _fixed_point_initial_norm;

    if (_has_fixed_point_norm && max_norm < _fixed_point_abs_tol)
    {
      _fp_solve.setFixedPointStatus(
          FixedPointSolve::MooseFixedPointConvergenceReason::CONVERGED_ABS);
      return MooseConvergenceStatus::CONVERGED;
    }
    if (_has_fixed_point_norm && max_relative_drop < _fixed_point_rel_tol)
    {
      _fp_solve.setFixedPointStatus(
          FixedPointSolve::MooseFixedPointConvergenceReason::CONVERGED_RELATIVE);
      return MooseConvergenceStatus::CONVERGED;
    }

    if (std::abs(_pp_new - _pp_old) < _custom_pp_abs_tol)
    {
      _fp_solve.setFixedPointStatus(
          FixedPointSolve::MooseFixedPointConvergenceReason::CONVERGED_PP);
      return MooseConvergenceStatus::CONVERGED;
    }
    if (std::abs((_pp_new - _pp_old) / _pp_scaling) < _custom_pp_rel_tol)
    {
      _fp_solve.setFixedPointStatus(
          FixedPointSolve::MooseFixedPointConvergenceReason::CONVERGED_PP);
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
DefaultMultiAppFixedPointConvergence::computeCustomConvergencePostprocessor(unsigned int iter)
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
