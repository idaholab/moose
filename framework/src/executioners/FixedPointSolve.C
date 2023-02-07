//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FixedPointSolve.h"

#include "FEProblem.h"
#include "Executioner.h"
#include "MooseMesh.h"
#include "NonlinearSystem.h"
#include "AllLocalDofIndicesThread.h"
#include "Console.h"
#include "EigenExecutionerBase.h"

InputParameters
FixedPointSolve::validParams()
{
  InputParameters params = emptyInputParameters();

  params.addParam<unsigned int>(
      "fixed_point_min_its", 1, "Specifies the minimum number of fixed point iterations.");
  params.addParam<unsigned int>(
      "fixed_point_max_its", 1, "Specifies the maximum number of fixed point iterations.");
  params.addParam<bool>(
      "accept_on_max_fixed_point_iteration",
      false,
      "True to treat reaching the maximum number of fixed point iterations as converged.");
  params.addParam<bool>("disable_fixed_point_residual_norm_check",
                        false,
                        "Disable the residual norm evaluation thus the three parameters "
                        "fixed_point_rel_tol, fixed_point_abs_tol and fixed_point_force_norms.");
  params.addRangeCheckedParam<Real>("fixed_point_rel_tol",
                                    1e-8,
                                    "fixed_point_rel_tol>0",
                                    "The relative nonlinear residual drop to shoot for "
                                    "during fixed point iterations. This check is "
                                    "performed based on the main app's nonlinear "
                                    "residual.");
  params.addRangeCheckedParam<Real>("fixed_point_abs_tol",
                                    1e-50,
                                    "fixed_point_abs_tol>0",
                                    "The absolute nonlinear residual to shoot for "
                                    "during fixed point iterations. This check is "
                                    "performed based on the main app's nonlinear "
                                    "residual.");
  params.addParam<bool>(
      "fixed_point_force_norms",
      false,
      "Force the evaluation of both the TIMESTEP_BEGIN and TIMESTEP_END norms regardless of the "
      "existence of active MultiApps with those execute_on flags, default: false.");

  // Parameters for using a custom postprocessor for convergence checks
  params.addParam<PostprocessorName>("custom_pp",
                                     "Postprocessor for custom fixed point convergence check.");
  params.addRangeCheckedParam<Real>("custom_rel_tol",
                                    1e-8,
                                    "custom_rel_tol>0",
                                    "The relative nonlinear residual drop to shoot for "
                                    "during fixed point iterations. This check is "
                                    "performed based on the postprocessor defined by "
                                    "custom_pp residual.");
  params.addRangeCheckedParam<Real>("custom_abs_tol",
                                    1e-50,
                                    "custom_abs_tol>0",
                                    "The absolute nonlinear residual to shoot for "
                                    "during fixed point iterations. This check is "
                                    "performed based on postprocessor defined by "
                                    "the custom_pp residual.");
  params.addParam<bool>("direct_pp_value",
                        false,
                        "True to use direct postprocessor value "
                        "(scaled by value on first iteration). "
                        "False (default) to use difference in postprocessor "
                        "value between fixed point iterations.");

  // Parameters for relaxing the fixed point process
  params.addRangeCheckedParam<Real>("relaxation_factor",
                                    1.0,
                                    "relaxation_factor>0 & relaxation_factor<2",
                                    "Fraction of newly computed value to keep."
                                    "Set between 0 and 2.");
  params.addParam<std::vector<std::string>>(
      "transformed_variables",
      std::vector<std::string>(),
      "List of main app variables to transform during fixed point iterations");
  params.addParam<std::vector<PostprocessorName>>(
      "transformed_postprocessors",
      std::vector<PostprocessorName>(),
      "List of main app postprocessors to transform during fixed point iterations");
  params.addDeprecatedParam<std::vector<std::string>>(
      "relaxed_variables",
      std::vector<std::string>(),
      "List of main app variables to relax during fixed point iterations",
      "Relaxed variables is deprecated, use transformed_variables instead.");

  params.addParam<bool>("auto_advance",
                        "Whether to automatically advance sub-applications regardless of whether "
                        "their solve converges, for transient executioners only.");

  params.addParamNamesToGroup(
      "fixed_point_min_its fixed_point_max_its accept_on_max_fixed_point_iteration "
      "disable_fixed_point_residual_norm_check fixed_point_rel_tol fixed_point_abs_tol "
      "fixed_point_force_norms custom_pp fixed_point_rel_tol fixed_point_abs_tol direct_pp_value "
      "relaxation_factor transformed_variables transformed_postprocessors auto_advance "
      "custom_abs_tol custom_rel_tol",
      "Fixed point iterations");

  params.addParam<unsigned int>(
      "max_xfem_update",
      std::numeric_limits<unsigned int>::max(),
      "Maximum number of times to update XFEM crack topology in a step due to evolving cracks");
  params.addParam<bool>("update_xfem_at_timestep_begin",
                        false,
                        "Should XFEM update the mesh at the beginning of the timestep");

  params.addParamNamesToGroup("max_xfem_update update_xfem_at_timestep_begin",
                              "XFEM fixed point iterations");

  return params;
}

FixedPointSolve::FixedPointSolve(Executioner & ex)
  : SolveObject(ex),
    _min_fixed_point_its(getParam<unsigned int>("fixed_point_min_its")),
    _max_fixed_point_its(getParam<unsigned int>("fixed_point_max_its")),
    _has_fixed_point_its(_max_fixed_point_its > 1),
    _accept_max_it(getParam<bool>("accept_on_max_fixed_point_iteration")),
    _has_fixed_point_norm(!getParam<bool>("disable_fixed_point_residual_norm_check")),
    _fixed_point_rel_tol(getParam<Real>("fixed_point_rel_tol")),
    _fixed_point_abs_tol(getParam<Real>("fixed_point_abs_tol")),
    _fixed_point_force_norms(getParam<bool>("fixed_point_force_norms")),
    _fixed_point_custom_pp(isParamValid("custom_pp") ? &getPostprocessorValue("custom_pp")
                                                     : nullptr),
    _relax_factor(getParam<Real>("relaxation_factor")),
    _transformed_vars(getParam<std::vector<std::string>>("transformed_variables")),
    _transformed_pps(getParam<std::vector<PostprocessorName>>("transformed_postprocessors")),
    // this value will be set by MultiApp
    _secondary_relaxation_factor(1.0),
    _fixed_point_it(0),
    _fixed_point_status(MooseFixedPointConvergenceReason::UNSOLVED),
    _custom_rel_tol(getParam<Real>("custom_rel_tol")),
    _custom_abs_tol(getParam<Real>("custom_abs_tol")),
    _pp_old(0.0),
    _pp_new(std::numeric_limits<Real>::max()),
    _pp_scaling(1),
    _max_xfem_update(getParam<unsigned int>("max_xfem_update")),
    _update_xfem_at_timestep_begin(getParam<bool>("update_xfem_at_timestep_begin")),
    _xfem_update_count(0),
    _xfem_repeat_step(false),
    _old_entering_time(_problem.time() - 1),
    _fail_step(false),
    _auto_advance_set_by_user(isParamValid("auto_advance")),
    _auto_advance_user_value(_auto_advance_set_by_user ? getParam<bool>("auto_advance") : true)
{
  // Handle deprecated parameters
  if (!parameters().isParamSetByAddParam("relaxed_variables"))
    _transformed_vars = getParam<std::vector<std::string>>("relaxed_variables");

  if (_min_fixed_point_its > _max_fixed_point_its)
    paramError("fixed_point_min_its",
               "The minimum number of fixed point iterations may not exceed the maximum.");

  if (_transformed_vars.size() > 0 && _transformed_pps.size() > 0)
    mooseWarning(
        "Both variable and postprocessor transformation are active. If the two share dofs, the "
        "transformation will not be correct.");

  if (!_app.isUltimateMaster())
  {
    _secondary_relaxation_factor = _app.fixedPointConfig().sub_relaxation_factor;
    _secondary_transformed_variables = _app.fixedPointConfig().sub_transformed_vars;
    _secondary_transformed_pps = _app.fixedPointConfig().sub_transformed_pps;
  }
}

bool
FixedPointSolve::solve()
{
  TIME_SECTION("PicardSolve", 1);

  Real current_dt = _problem.dt();

  _fixed_point_timestep_begin_norm.clear();
  _fixed_point_timestep_end_norm.clear();
  _fixed_point_timestep_begin_norm.resize(_max_fixed_point_its);
  _fixed_point_timestep_end_norm.resize(_max_fixed_point_its);

  bool converged = true;

  // need to back up multi-apps even when not doing fixed point iteration for recovering from failed
  // multiapp solve
  _problem.backupMultiApps(EXEC_MULTIAPP_FIXED_POINT_BEGIN);
  _problem.backupMultiApps(EXEC_TIMESTEP_BEGIN);
  _problem.backupMultiApps(EXEC_TIMESTEP_END);
  _problem.backupMultiApps(EXEC_MULTIAPP_FIXED_POINT_END);

  // Prepare to relax variables as a main app
  std::set<dof_id_type> transformed_dofs;
  if ((_relax_factor != 1.0 || !dynamic_cast<PicardSolve *>(this)) && _transformed_vars.size() > 0)
  {
    // Snag all of the local dof indices for all of these variables
    AllLocalDofIndicesThread aldit(_problem, _transformed_vars);
    ConstElemRange & elem_range = *_problem.mesh().getActiveLocalElementRange();
    Threads::parallel_reduce(elem_range, aldit);

    transformed_dofs = aldit.getDofIndices();
  }

  // Prepare to relax variables as a subapp
  std::set<dof_id_type> secondary_transformed_dofs;
  if (_secondary_relaxation_factor != 1.0 || !dynamic_cast<PicardSolve *>(this))
  {
    if (_secondary_transformed_variables.size() > 0)
    {
      // Snag all of the local dof indices for all of these variables
      AllLocalDofIndicesThread aldit(_problem, _secondary_transformed_variables);
      ConstElemRange & elem_range = *_problem.mesh().getActiveLocalElementRange();
      Threads::parallel_reduce(elem_range, aldit);

      secondary_transformed_dofs = aldit.getDofIndices();
    }

    // To detect a new time step
    if (_old_entering_time == _problem.time())
    {
      // Keep track of the iteration number of the main app
      _main_fixed_point_it++;

      // Save variable values before the solve. Solving will provide new values
      saveVariableValues(false);
    }
    else
      _main_fixed_point_it = 0;
  }

  for (_fixed_point_it = 0; _fixed_point_it < _max_fixed_point_its; ++_fixed_point_it)
  {
    if (_has_fixed_point_its)
    {
      if (_fixed_point_it == 0)
      {
        if (_has_fixed_point_norm)
        {
          // First fixed point iteration - need to save off the initial nonlinear residual
          _fixed_point_initial_norm = _problem.computeResidualL2Norm();
          _console << COLOR_MAGENTA << "Initial fixed point residual norm: " << COLOR_DEFAULT;
          if (_fixed_point_initial_norm == std::numeric_limits<Real>::max())
            _console << " MAX ";
          else
            _console << std::scientific << _fixed_point_initial_norm;
          _console << COLOR_DEFAULT << "\n" << std::endl;
        }
      }
      else
      {
        // For every iteration other than the first, we need to restore the state of the MultiApps
        _problem.restoreMultiApps(EXEC_MULTIAPP_FIXED_POINT_BEGIN);
        _problem.restoreMultiApps(EXEC_TIMESTEP_BEGIN);
        _problem.restoreMultiApps(EXEC_TIMESTEP_END);
        _problem.restoreMultiApps(EXEC_MULTIAPP_FIXED_POINT_END);
      }

      _console << COLOR_MAGENTA << "Beginning fixed point iteration " << _fixed_point_it
               << COLOR_DEFAULT << std::endl
               << std::endl;
    }

    // Save last postprocessor value as value before solve
    if (_fixed_point_custom_pp && _fixed_point_it > 0 && !getParam<bool>("direct_pp_value"))
      _pp_old = *_fixed_point_custom_pp;

    // Solve a single application for one time step
    bool solve_converged = solveStep(_fixed_point_timestep_begin_norm[_fixed_point_it],
                                     _fixed_point_timestep_end_norm[_fixed_point_it],
                                     transformed_dofs);

    // Get new value and print history for the custom postprocessor convergence criterion
    if (_fixed_point_custom_pp)
      computeCustomConvergencePostprocessor();

    if (solve_converged)
    {
      if (_has_fixed_point_its)
      {
        if (_has_fixed_point_norm)
          // Print the evolution of the main app residual over the fixed point iterations
          printFixedPointConvergenceHistory();

        // Examine convergence metrics & properties and set the convergence reason
        bool break_out = examineFixedPointConvergence(converged);
        if (break_out)
          break;
      }
    }
    else
    {
      // If the last solve didn't converge then we need to exit this step completely (even in the
      // case of coupling). So we can retry...
      converged = false;
      break;
    }

    if (converged)
    {
      // Fixed point iteration loop ends right above
      _problem.execute(EXEC_MULTIAPP_FIXED_POINT_END);
      _problem.execTransfers(EXEC_MULTIAPP_FIXED_POINT_END);
      if (!_problem.execMultiApps(EXEC_MULTIAPP_FIXED_POINT_END, autoAdvance()))
      {
        _fixed_point_status = MooseFixedPointConvergenceReason::DIVERGED_FAILED_MULTIAPP;
        return false;
      }
      _problem.outputStep(EXEC_MULTIAPP_FIXED_POINT_END);
    }

    _problem.dt() =
        current_dt; // _dt might be smaller than this at this point for multistep methods
  }

  // Save postprocessors after the solve and their potential timestep_end execution
  // The postprocessors could be overwritten at timestep_begin, which is why they are saved
  // after the solve. They could also be saved right after the transfers.
  if (_old_entering_time == _problem.time())
    savePostprocessorValues(false);

  if (converged)
  {
    // Update the subapp using the fixed point algorithm
    if (_secondary_transformed_variables.size() > 0 &&
        useFixedPointAlgorithmUpdateInsteadOfPicard(false) && _old_entering_time == _problem.time())
      transformVariables(secondary_transformed_dofs, false);

    // Update the entering time, used to detect failed solves
    _old_entering_time = _problem.time();
  }

  if (_has_fixed_point_its)
    printFixedPointConvergenceReason();

  // clear history to avoid displaying it again on next solve that can happen for example during
  // transient
  _pp_history.str("");

  return converged;
}

void
FixedPointSolve::saveAllValues(const bool primary)
{
  saveVariableValues(primary);
  savePostprocessorValues(primary);
}

bool
FixedPointSolve::solveStep(Real & begin_norm,
                           Real & end_norm,
                           const std::set<dof_id_type> & transformed_dofs)
{
  bool auto_advance = autoAdvance();

  // Compute previous norms for coloring the norm output
  Real begin_norm_old = (_fixed_point_it > 0 ? _fixed_point_timestep_begin_norm[_fixed_point_it - 1]
                                             : std::numeric_limits<Real>::max());
  Real end_norm_old = (_fixed_point_it > 0 ? _fixed_point_timestep_end_norm[_fixed_point_it - 1]
                                           : std::numeric_limits<Real>::max());

  _executioner.preSolve();
  _problem.execTransfers(EXEC_TIMESTEP_BEGIN);

  if (_fixed_point_it == 0)
  {
    _problem.execute(EXEC_MULTIAPP_FIXED_POINT_BEGIN);
    _problem.execTransfers(EXEC_MULTIAPP_FIXED_POINT_BEGIN);
    if (!_problem.execMultiApps(EXEC_MULTIAPP_FIXED_POINT_BEGIN, autoAdvance()))
    {
      _fixed_point_status = MooseFixedPointConvergenceReason::DIVERGED_FAILED_MULTIAPP;
      return false;
    }
    _problem.outputStep(EXEC_MULTIAPP_FIXED_POINT_BEGIN);
  }

  if (!_problem.execMultiApps(EXEC_TIMESTEP_BEGIN, auto_advance))
  {
    _fixed_point_status = MooseFixedPointConvergenceReason::DIVERGED_FAILED_MULTIAPP;
    return false;
  }

  if (_problem.haveXFEM() && _update_xfem_at_timestep_begin)
    _problem.updateMeshXFEM();

  _problem.execute(EXEC_TIMESTEP_BEGIN);

  // Transform the fixed point postprocessors before solving, but after the timestep_begin transfers
  // have been received
  if (_transformed_pps.size() > 0 && useFixedPointAlgorithmUpdateInsteadOfPicard(true))
    transformPostprocessors(true);
  if (_secondary_transformed_pps.size() > 0 && useFixedPointAlgorithmUpdateInsteadOfPicard(false) &&
      _problem.time() == _old_entering_time)
    transformPostprocessors(false);

  if (_has_fixed_point_its && _has_fixed_point_norm)
    if (_problem.hasMultiApps(EXEC_TIMESTEP_BEGIN) || _fixed_point_force_norms)
    {
      begin_norm = _problem.computeResidualL2Norm();

      _console << COLOR_MAGENTA << "Fixed point residual norm after TIMESTEP_BEGIN MultiApps: "
               << Console::outputNorm(begin_norm_old, begin_norm) << std::endl;
    }

  // Perform output for timestep begin
  _problem.outputStep(EXEC_TIMESTEP_BEGIN);

  // Update warehouse active objects
  _problem.updateActiveObjects();

  // Save the current values of variables and postprocessors, before the solve
  saveAllValues(true);

  if (_has_fixed_point_its)
    _console << COLOR_MAGENTA << "\nMain app solve:" << COLOR_DEFAULT << std::endl;
  if (!_inner_solve->solve())
  {
    _fixed_point_status = MooseFixedPointConvergenceReason::DIVERGED_NONLINEAR;

    // Perform the output of the current, failed time step (this only occurs if desired)
    _problem.outputStep(EXEC_FAILED);
    return false;
  }
  else
    _fixed_point_status = MooseFixedPointConvergenceReason::CONVERGED_NONLINEAR;

  // Use the fixed point algorithm if the conditions (availability of values, etc) are met
  if (_transformed_vars.size() > 0 && useFixedPointAlgorithmUpdateInsteadOfPicard(true))
    transformVariables(transformed_dofs, true);

  if (_problem.haveXFEM() && (_xfem_update_count < _max_xfem_update) && _problem.updateMeshXFEM())
  {
    _console << "\nXFEM modified mesh, repeating step" << std::endl;
    _xfem_repeat_step = true;
    ++_xfem_update_count;
  }
  else
  {
    if (_problem.haveXFEM())
    {
      _xfem_repeat_step = false;
      _xfem_update_count = 0;
      _console << "\nXFEM did not modify mesh, continuing" << std::endl;
    }

    _problem.onTimestepEnd();
    _problem.execute(EXEC_TIMESTEP_END);

    _problem.execTransfers(EXEC_TIMESTEP_END);
    if (!_problem.execMultiApps(EXEC_TIMESTEP_END, auto_advance))
    {
      _fixed_point_status = MooseFixedPointConvergenceReason::DIVERGED_FAILED_MULTIAPP;
      return false;
    }
  }

  if (_fail_step)
  {
    _fail_step = false;
    return false;
  }

  _executioner.postSolve();

  if (_has_fixed_point_its && _has_fixed_point_norm)
    if (_problem.hasMultiApps(EXEC_TIMESTEP_END) || _fixed_point_force_norms)
    {
      end_norm = _problem.computeResidualL2Norm();

      _console << COLOR_MAGENTA << "Fixed point residual norm after TIMESTEP_END MultiApps: "
               << Console::outputNorm(end_norm_old, end_norm) << std::endl;
    }

  return true;
}

void
FixedPointSolve::computeCustomConvergencePostprocessor()
{
  if ((_fixed_point_it == 0 && getParam<bool>("direct_pp_value")) ||
      !getParam<bool>("direct_pp_value"))
    _pp_scaling = *_fixed_point_custom_pp;
  _pp_new = *_fixed_point_custom_pp;

  auto ppname = getParam<PostprocessorName>("custom_pp");
  _pp_history << std::setw(2) << _fixed_point_it + 1 << " fixed point " << ppname << " = "
              << Console::outputNorm(std::numeric_limits<Real>::max(), _pp_new, 8) << std::endl;
  _console << _pp_history.str();
}

bool
FixedPointSolve::examineFixedPointConvergence(bool & converged)
{
  if (_fixed_point_it + 2 > _min_fixed_point_its)
  {
    Real max_norm = std::max(_fixed_point_timestep_begin_norm[_fixed_point_it],
                             _fixed_point_timestep_end_norm[_fixed_point_it]);

    Real max_relative_drop = max_norm / _fixed_point_initial_norm;

    if (_has_fixed_point_norm && max_norm < _fixed_point_abs_tol)
    {
      _fixed_point_status = MooseFixedPointConvergenceReason::CONVERGED_ABS;
      return true;
    }
    if (_has_fixed_point_norm && max_relative_drop < _fixed_point_rel_tol)
    {
      _fixed_point_status = MooseFixedPointConvergenceReason::CONVERGED_RELATIVE;
      return true;
    }
  }
  if (std::abs(_pp_new - _pp_old) < _custom_abs_tol)
  {
    _fixed_point_status = MooseFixedPointConvergenceReason::CONVERGED_CUSTOM;
    return true;
  }
  if (std::abs((_pp_new - _pp_old) / _pp_scaling) < _custom_rel_tol)
  {
    _fixed_point_status = MooseFixedPointConvergenceReason::CONVERGED_CUSTOM;
    return true;
  }
  if (_fixed_point_it + 1 == _max_fixed_point_its)
  {
    if (_accept_max_it)
    {
      _fixed_point_status = MooseFixedPointConvergenceReason::REACH_MAX_ITS;
      converged = true;
    }
    else
    {
      _fixed_point_status = MooseFixedPointConvergenceReason::DIVERGED_MAX_ITS;
      converged = false;
    }
    return true;
  }
  return false;
}

void
FixedPointSolve::printFixedPointConvergenceReason()
{
  _console << "Fixed point convergence reason: ";
  switch (_fixed_point_status)
  {
    case MooseFixedPointConvergenceReason::CONVERGED_ABS:
      _console << "CONVERGED_ABS";
      break;
    case MooseFixedPointConvergenceReason::CONVERGED_RELATIVE:
      _console << "CONVERGED_RELATIVE";
      break;
    case MooseFixedPointConvergenceReason::CONVERGED_CUSTOM:
      _console << "CONVERGED_CUSTOM";
      break;
    case MooseFixedPointConvergenceReason::REACH_MAX_ITS:
      _console << "REACH_MAX_ITS";
      break;
    case MooseFixedPointConvergenceReason::DIVERGED_MAX_ITS:
      _console << "DIVERGED_MAX_ITS";
      break;
    case MooseFixedPointConvergenceReason::DIVERGED_NONLINEAR:
      _console << "DIVERGED_NONLINEAR";
      break;
    case MooseFixedPointConvergenceReason::DIVERGED_FAILED_MULTIAPP:
      _console << "DIVERGED_FAILED_MULTIAPP";
      break;
    default:
      // UNSOLVED and CONVERGED_NONLINEAR should not be hit when coupling
      // iteration is not on here
      mooseError("Internal error: wrong fixed point status!");
      break;
  }
  _console << std::endl;
}

bool
FixedPointSolve::autoAdvance() const
{
  bool auto_advance = !(_has_fixed_point_its && _problem.isTransient());

  if (dynamic_cast<EigenExecutionerBase *>(&_executioner) && _has_fixed_point_its)
    auto_advance = true;

  if (_auto_advance_set_by_user)
    auto_advance = _auto_advance_user_value;

  return auto_advance;
}
