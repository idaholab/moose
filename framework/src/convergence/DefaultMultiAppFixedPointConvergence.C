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
#include "SteffensenSolve.h"

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
    _min_fixed_point_its(getSharedExecutionerParam<unsigned int>("fixed_point_min_its")),
    _max_fixed_point_its(getSharedExecutionerParam<unsigned int>("fixed_point_max_its")),
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
  if (_min_fixed_point_its > _max_fixed_point_its)
    paramError("fixed_point_min_its",
               "The minimum number of fixed point iterations may not exceed the maximum.");
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

  if (dynamic_cast<SteffensenSolve *>(&_fp_solve) && _fe_problem.hasMultiApps())
  {
    // Steffensen method uses half-steps
    if (!parameters.isParamSetByAddParam("fixed_point_min_its"))
      _min_fixed_point_its *= 2;
    _max_fixed_point_its *= 2;
  }
}

void
DefaultMultiAppFixedPointConvergence::checkIterationType(IterationType it_type) const
{
  DefaultConvergenceBase::checkIterationType(it_type);

  if (it_type != IterationType::MULTIAPP_FIXED_POINT)
    mooseError(
        "DefaultMultiAppFixedPointConvergence can only be used with MultiApp fixed point solves.");
}

void
DefaultMultiAppFixedPointConvergence::initialize()
{
  DefaultConvergenceBase::initialize();

  _fixed_point_timestep_begin_norm.clear();
  _fixed_point_timestep_end_norm.clear();
  _fixed_point_timestep_begin_norm.resize(_max_fixed_point_its);
  _fixed_point_timestep_end_norm.resize(_max_fixed_point_its);

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

  // compute TIMESTEP_BEGIN residual norm; this should be executed after TIMESTEP_BEGIN
  // but before the solve
  if (_has_fixed_point_norm &&
      (_fe_problem.hasMultiApps(EXEC_TIMESTEP_BEGIN) || _fixed_point_force_norms))
  {
    const auto iter = _fp_solve.numFixedPointIts() - 1;
    _fixed_point_timestep_begin_norm[iter] = _fe_problem.computeResidualL2Norm();

    Real begin_norm_old =
        (iter > 0 ? _fixed_point_timestep_begin_norm[iter - 1] : std::numeric_limits<Real>::max());

    outputResidualNorm("TIMESTEP_BEGIN", begin_norm_old, _fixed_point_timestep_begin_norm[iter]);
  }
}

Convergence::MooseConvergenceStatus
DefaultMultiAppFixedPointConvergence::checkConvergence(unsigned int iter)
{
  TIME_SECTION(_perfid_check_convergence);

  if (_fixed_point_custom_pp)
    computeCustomConvergencePostprocessor(iter);

  // compute TIMESTEP_END residual norm; this should be executed between TIMESTEP_END
  // and TIMESTEP_BEGIN
  if (_has_fixed_point_norm)
    if (_fe_problem.hasMultiApps(EXEC_TIMESTEP_END) || _fixed_point_force_norms)
    {
      _fixed_point_timestep_end_norm[iter] = _fe_problem.computeResidualL2Norm();

      Real end_norm_old =
          (iter > 0 ? _fixed_point_timestep_end_norm[iter - 1] : std::numeric_limits<Real>::max());

      outputResidualNorm("TIMESTEP_END", end_norm_old, _fixed_point_timestep_end_norm[iter]);
    }

  // print residual norm history
  if (_has_fixed_point_norm)
    _fp_solve.printFixedPointConvergenceHistory(_fixed_point_initial_norm,
                                                _fixed_point_timestep_begin_norm,
                                                _fixed_point_timestep_end_norm);

  if (iter + 2 > _min_fixed_point_its)
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

  if (iter + 1 == _max_fixed_point_its)
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
DefaultMultiAppFixedPointConvergence::outputResidualNorm(const std::string & execute_on_str,
                                                         Real old_norm,
                                                         Real new_norm) const
{
  _console << COLOR_MAGENTA << "Fixed point residual norm after " << execute_on_str
           << " MultiApps: " << Console::outputNorm(old_norm, new_norm) << std::endl;
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
