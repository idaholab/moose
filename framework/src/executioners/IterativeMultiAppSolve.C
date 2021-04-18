//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IterativeMultiAppSolve.h"

#include "FEProblem.h"
#include "Executioner.h"
#include "MooseMesh.h"
#include "NonlinearSystem.h"
#include "AllLocalDofIndicesThread.h"
#include "Console.h"
#include "EigenExecutionerBase.h"

InputParameters
IterativeMultiAppSolve::validParams()
{
  InputParameters params = emptyInputParameters();

  params.addParam<unsigned int>(
      "coupling_min_its", 0, "Specifies the minimum number of coupling iterations.");
  params.addParam<unsigned int>(
      "coupling_max_its", 1, "Specifies the maximum number of coupling iterations.");
  params.addParam<bool>(
      "accept_on_max_coupling_iteration",
      false,
      "True to treat reaching the maximum number of coupling iterations as converged.");
  params.addParam<bool>("disable_coupling_residual_norm_check",
                        false,
                        "Disable the residual norm evaluation thus the three parameters "
                        "coupling_rel_tol, coupling_abs_tol and coupling_force_norms.");
  params.addParam<Real>("coupling_rel_tol",
                        1e-8,
                        "The relative nonlinear residual drop to shoot for "
                        "during coupling iterations. This check is "
                        "performed based on the main app's nonlinear "
                        "residual.");
  params.addParam<Real>("coupling_abs_tol",
                        1e-50,
                        "The absolute nonlinear residual to shoot for "
                        "during coupling iterations. This check is "
                        "performed based on the main app's nonlinear "
                        "residual.");
  params.addParam<bool>(
      "coupling_force_norms",
      false,
      "Force the evaluation of both the TIMESTEP_BEGIN and TIMESTEP_END norms regardless of the "
      "existence of active MultiApps with those execute_on flags, default: false.");

  // Parameters for using a custom postprocessor for convergence checks
  params.addParam<PostprocessorName>("coupling_custom_pp",
                                     "Postprocessor for custom coupling convergence check.");
  params.addParam<Real>("custom_rel_tol",
                        1e-8,
                        "The relative nonlinear residual drop to shoot for "
                        "during coupling iterations. This check is "
                        "performed based on the postprocessor defined by "
                        "coupling_custom_pp residual.");
  params.addParam<Real>("custom_abs_tol",
                        1e-50,
                        "The absolute nonlinear residual to shoot for "
                        "during coupling iterations. This check is "
                        "performed based on postprocessor defined by "
                        "the coupling_custom_pp residual.");
  params.addParam<bool>("direct_pp_value",
                        false,
                        "True to use direct postprocessor value "
                        "(scaled by value on first iteration). "
                        "False (default) to use difference in postprocessor "
                        "value between coupling iterations.");

  // Parameters for relaxing the coupling process
  params.addRangeCheckedParam<Real>("relaxation_factor",
                                    1.0,
                                    "relaxation_factor>0 & relaxation_factor<2",
                                    "Fraction of newly computed value to keep."
                                    "Set between 0 and 2.");
  params.addParam<std::vector<std::string>>(
      "transformed_variables",
      std::vector<std::string>(),
      "List of main app variables to transform during coupling iterations");
  params.addParam<std::vector<std::string>>(
      "transformed_postprocessors",
      std::vector<std::string>(),
      "List of main app postprocessors to transform during coupling iterations");
  params.addDeprecatedParam<std::vector<std::string>>(
      "relaxed_variables",
      std::vector<std::string>(),
      "Relaxed variables is deprecated, use transformed_variables instead.",
      "List of master app variables to relax during coupling iterations");

  params.addParam<bool>("auto_advance",
                        "Whether to automatically advance sub-applications regardless of whether "
                        "their solve converges.");

  params.addParamNamesToGroup(
      "coupling_min_its coupling_max_its accept_on_max_coupling_iteration "
      "disable_coupling_residual_norm_check coupling_rel_tol coupling_abs_tol "
      "coupling_force_norms coupling_custom_pp coupling_rel_tol coupling_abs_tol direct_pp_value "
      "relaxation_factor transformed_variables transformed_postprocessors auto_advance",
      "Multiapp coupling");

  params.addParam<unsigned int>(
      "max_xfem_update",
      std::numeric_limits<unsigned int>::max(),
      "Maximum number of times to update XFEM crack topology in a step due to evolving cracks");
  params.addParam<bool>("update_xfem_at_timestep_begin",
                        false,
                        "Should XFEM update the mesh at the beginning of the timestep");

  params.addParamNamesToGroup("max_xfem_update update_xfem_at_timestep_begin",
                              "XFEM multiapp coupling");

  return params;
}

IterativeMultiAppSolve::IterativeMultiAppSolve(Executioner * ex)
  : SolveObject(ex),
    _coupling_min_its(getParam<unsigned int>("coupling_min_its")),
    _coupling_max_its(getParam<unsigned int>("coupling_max_its")),
    _has_coupling_its(_coupling_max_its > 1),
    _accept_max_it(getParam<bool>("accept_on_max_coupling_iteration")),
    _has_coupling_norm(!getParam<bool>("disable_coupling_residual_norm_check")),
    _coupling_rel_tol(getParam<Real>("coupling_rel_tol")),
    _coupling_abs_tol(getParam<Real>("coupling_abs_tol")),
    _coupling_force_norms(getParam<bool>("coupling_force_norms")),

    _coupling_custom_pp(isParamValid("coupling_custom_pp")
                            ? &getPostprocessorValue("coupling_custom_pp")
                            : nullptr),
    _custom_rel_tol(getParam<Real>("custom_rel_tol")),
    _custom_abs_tol(getParam<Real>("custom_abs_tol")),
    _pp_old(0.0),
    _pp_new(std::numeric_limits<Real>::max()),
    _pp_scaling(1),

    _relax_factor(getParam<Real>("relaxation_factor")),
    _transformed_vars(getParam<std::vector<std::string>>("transformed_variables")),
    _transformed_pps(getParam<std::vector<std::string>>("transformed_postprocessors")),
    // this value will be set by MultiApp
    _secondary_relaxation_factor(1.0),
    _max_xfem_update(getParam<unsigned int>("max_xfem_update")),
    _update_xfem_at_timestep_begin(getParam<bool>("update_xfem_at_timestep_begin")),
    _coupling_timer(registerTimedSection("IterativeMultiAppSolve", 1)),
    _coupling_it(0),
    _coupling_status(MooseCouplingConvergenceReason::UNSOLVED),
    _xfem_update_count(0),
    _xfem_repeat_step(false),
    _old_entering_time(_problem.time() - 1),
    _solve_message(_problem.shouldSolve() ? "Solve Converged!" : "Solve Skipped!"),
    _fail_step(false),
    _auto_advance_set_by_user(isParamValid("auto_advance")),
    _auto_advance_user_value(_auto_advance_set_by_user ? getParam<bool>("auto_advance") : true)
{
  // Handle deprecated parameters
  if (!parameters().isParamSetByAddParam("relaxed_variables"))
    _transformed_vars = getParam<std::vector<std::string>>("relaxed_variables");

  if (_coupling_min_its > _coupling_max_its)
    paramError("coupling_min_its",
               "The minimum number of coupling iterations may not exceed the maximum.");

  if (_transformed_vars.size() > 0 && _transformed_pps.size() > 0)
    mooseWarning(
        "Both variable and postprocessor transformation are active. If the two share dofs, the "
        "transformation will not be correct.");
}

bool
IterativeMultiAppSolve::solve()
{
  TIME_SECTION(_coupling_timer);

  Real current_dt = _problem.dt();

  _coupling_timestep_begin_norm.clear();
  _coupling_timestep_end_norm.clear();
  _coupling_timestep_begin_norm.resize(_coupling_max_its);
  _coupling_timestep_end_norm.resize(_coupling_max_its);

  bool converged = true;

  // need to back up multi-apps even when not doing coupling iteration for recovering from failed
  // multiapp solve
  _problem.backupMultiApps(EXEC_TIMESTEP_BEGIN);
  _problem.backupMultiApps(EXEC_TIMESTEP_END);

  // Prepare to relax variables as a main app
  std::set<dof_id_type> transformed_dofs;
  if (_relax_factor != 1.0 || !dynamic_cast<PicardSolve *>(this))
  {
    // Snag all of the local dof indices for all of these variables
    System & libmesh_nl_system = _nl.system();
    AllLocalDofIndicesThread aldit(libmesh_nl_system, _transformed_vars);
    ConstElemRange & elem_range = *_problem.mesh().getActiveLocalElementRange();
    Threads::parallel_reduce(elem_range, aldit);

    transformed_dofs = aldit.getDofIndices();
  }

  // Prepare to relax variables as a subapp
  std::set<dof_id_type> secondary_transformed_dofs;
  if (_secondary_relaxation_factor != 1.0 || !dynamic_cast<PicardSolve *>(this))
  {
    // Snag all of the local dof indices for all of these variables
    System & libmesh_nl_system = _nl.system();
    AllLocalDofIndicesThread aldit(libmesh_nl_system, _secondary_transformed_variables);
    ConstElemRange & elem_range = *_problem.mesh().getActiveLocalElementRange();
    Threads::parallel_reduce(elem_range, aldit);

    secondary_transformed_dofs = aldit.getDofIndices();

    // To detect a new time step
    if (_old_entering_time == _problem.time())
    {
      // Keep track of the iteration number of the main app
      _main_coupling_it++;

      // Save variable values before the solve. Solving will provide new values
      savePreviousVariableValuesAsSubApp();
    }
    else
      _main_coupling_it = 0;
  }

  for (_coupling_it = 0; _coupling_it < _coupling_max_its; ++_coupling_it)
  {
    if (_has_coupling_its)
    {
      if (_coupling_it == 0)
      {
        if (_has_coupling_norm)
        {
          // First coupling iteration - need to save off the initial nonlinear residual
          _coupling_initial_norm = _problem.computeResidualL2Norm();
          _console << COLOR_MAGENTA << "Initial coupling residual norm: " << COLOR_DEFAULT;
          if (_coupling_initial_norm == std::numeric_limits<Real>::max())
            _console << " MAX ";
          else
            _console << std::scientific << _coupling_initial_norm;
          _console << COLOR_DEFAULT << "\n\n";
        }
      }
      else
      {
        // For every iteration other than the first, we need to restore the state of the MultiApps
        _problem.restoreMultiApps(EXEC_TIMESTEP_BEGIN);
        _problem.restoreMultiApps(EXEC_TIMESTEP_END);
      }

      _console << COLOR_MAGENTA << "Beginning coupling Iteration " << _coupling_it << COLOR_DEFAULT
               << '\n';
    }

    // Save last postprocessor value as value before solve
    if (_coupling_custom_pp && _coupling_it > 0 && !getParam<bool>("direct_pp_value"))
      _pp_old = *_coupling_custom_pp;

    // Solve a single application for one time step
    bool solve_converged = solveStep(_coupling_timestep_begin_norm[_coupling_it],
                                     _coupling_timestep_end_norm[_coupling_it],
                                     transformed_dofs);

    // Get new value and print history for the custom postprocessor convergence criterion
    if (_coupling_custom_pp)
      computeCustomConvergencePostprocessor();

    if (solve_converged)
    {
      if (_has_coupling_its)
      {
        if (_has_coupling_norm)
          // Print the evolution of the main app residual over the coupling iterations
          printCouplingConvergenceHistory();

        // Examine convergence metrics & properties and set the convergence reason
        bool break_out = examineCouplingConvergence(converged);
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

    _problem.dt() =
        current_dt; // _dt might be smaller than this at this point for multistep methods
  }

  // Save postprocessors after the solve and their potential timestep_end execution
  // The postprocessors could be overwritten at timestep_begin, which is why they are saved now
  if (_old_entering_time == _problem.time())
    savePreviousPostprocessorValuesAsSubApp();

  if (converged)
  {
    // Update the subapp using the coupling algorithm
    if (_secondary_transformed_variables.size() > 0 && useCouplingAlgorithmUpdate(false) &&
        _old_entering_time == _problem.time())
      transformVariablesAsSubApp(secondary_transformed_dofs);

    // Update the entering time, used to detect failed solves
    _old_entering_time = _problem.time();
  }

  if (_has_coupling_its)
    printCouplingConvergenceReason();

  return converged;
}

bool
IterativeMultiAppSolve::solveStep(Real & begin_norm,
                                  Real & end_norm,
                                  const std::set<dof_id_type> & transformed_dofs)
{
  bool auto_advance = autoAdvance();

  // Compute previous norms for coloring the norm output
  Real begin_norm_old = (_coupling_it > 0 ? _coupling_timestep_begin_norm[_coupling_it - 1]
                                          : std::numeric_limits<Real>::max());
  Real end_norm_old = (_coupling_it > 0 ? _coupling_timestep_end_norm[_coupling_it - 1]
                                        : std::numeric_limits<Real>::max());

  _executioner.preSolve();

  _problem.execTransfers(EXEC_TIMESTEP_BEGIN);
  if (!_problem.execMultiApps(EXEC_TIMESTEP_BEGIN, auto_advance))
  {
    _coupling_status = MooseCouplingConvergenceReason::DIVERGED_FAILED_MULTIAPP;
    return false;
  }

  // Transform the coupling postprocessors before solving, but after the timestep_begin transfers
  // have been received
  if (_transformed_pps.size() > 0 && useCouplingAlgorithmUpdate(true))
    transformPostprocessorsAsMainApp();
  if (_secondary_transformed_pps.size() > 0 && useCouplingAlgorithmUpdate(false) &&
      _problem.time() == _old_entering_time)
    transformPostprocessorsAsSubApp();

  if (_problem.haveXFEM() && _update_xfem_at_timestep_begin)
    _problem.updateMeshXFEM();

  _problem.execute(EXEC_TIMESTEP_BEGIN);

  if (_has_coupling_its && _has_coupling_norm)
    if (_problem.hasMultiApps(EXEC_TIMESTEP_BEGIN) || _coupling_force_norms)
    {
      begin_norm = _problem.computeResidualL2Norm();

      _console << COLOR_MAGENTA << "Coupling residual norm after TIMESTEP_BEGIN MultiApps: "
               << Console::outputNorm(begin_norm_old, begin_norm) << '\n';
    }

  // Perform output for timestep begin
  _problem.outputStep(EXEC_TIMESTEP_BEGIN);

  // Update warehouse active objects
  _problem.updateActiveObjects();

  // Save previous solutions
  savePreviousValuesAsMainApp();

  if (_has_coupling_its)
    _console << COLOR_MAGENTA << "\nMain app solve:\n" << COLOR_DEFAULT;
  if (!_inner_solve->solve())
  {
    _coupling_status = MooseCouplingConvergenceReason::DIVERGED_NONLINEAR;

    _console << COLOR_RED << " Solve Did NOT Converge!" << COLOR_DEFAULT << std::endl;
    // Perform the output of the current, failed time step (this only occurs if desired)
    _problem.outputStep(EXEC_FAILED);
    return false;
  }
  else
    _coupling_status = MooseCouplingConvergenceReason::CONVERGED_NONLINEAR;

  _console << COLOR_GREEN << ' ' << _solve_message << COLOR_DEFAULT << std::endl;

  // Use the coupling algorithm if the conditions (availability of values, etc) are met
  if (_transformed_vars.size() > 0 && useCouplingAlgorithmUpdate(true))
    transformVariablesAsMainApp(transformed_dofs);

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
      _coupling_status = MooseCouplingConvergenceReason::DIVERGED_FAILED_MULTIAPP;
      return false;
    }
  }

  if (_fail_step)
  {
    _fail_step = false;
    return false;
  }

  _executioner.postSolve();

  if (_has_coupling_its && _has_coupling_norm)
    if (_problem.hasMultiApps(EXEC_TIMESTEP_END) || _coupling_force_norms)
    {
      end_norm = _problem.computeResidualL2Norm();

      _console << COLOR_MAGENTA << "Coupling iteration residual after TIMESTEP_END MultiApps: "
               << Console::outputNorm(end_norm_old, end_norm) << '\n';
    }

  return true;
}

void
IterativeMultiAppSolve::computeCustomConvergencePostprocessor()
{
  if ((_coupling_it == 0 && getParam<bool>("direct_pp_value")) ||
      !getParam<bool>("direct_pp_value"))
    _pp_scaling = *_coupling_custom_pp;
  _pp_new = *_coupling_custom_pp;

  auto ppname = getParam<PostprocessorName>("coupling_custom_pp");
  _pp_history << std::setw(2) << _coupling_it + 1 << " coupling " << ppname << " = "
              << Console::outputNorm(std::numeric_limits<Real>::max(), _pp_new) << "\n";
  _console << _pp_history.str();
}

bool
IterativeMultiAppSolve::examineCouplingConvergence(bool & converged)
{
  if (_coupling_it + 2 > _coupling_min_its)
  {
    Real max_norm = std::max(_coupling_timestep_begin_norm[_coupling_it],
                             _coupling_timestep_end_norm[_coupling_it]);

    Real max_relative_drop = max_norm / _coupling_initial_norm;

    if (_has_coupling_norm && max_norm < _coupling_abs_tol)
    {
      _coupling_status = MooseCouplingConvergenceReason::CONVERGED_ABS;
      return true;
    }
    if (_has_coupling_norm && max_relative_drop < _coupling_rel_tol)
    {
      _coupling_status = MooseCouplingConvergenceReason::CONVERGED_RELATIVE;
      return true;
    }
  }
  if (_executioner.augmentedCouplingConvergenceCheck())
  {
    _coupling_status = MooseCouplingConvergenceReason::CONVERGED_CUSTOM;
    return true;
  }
  if (std::abs(_pp_new - _pp_old) < _custom_abs_tol)
  {
    _coupling_status = MooseCouplingConvergenceReason::CONVERGED_CUSTOM;
    return true;
  }
  if (std::abs((_pp_new - _pp_old) / _pp_scaling) < _custom_rel_tol)
  {
    _coupling_status = MooseCouplingConvergenceReason::CONVERGED_CUSTOM;
    return true;
  }
  if (_coupling_it + 1 == _coupling_max_its)
  {
    if (_accept_max_it)
    {
      _coupling_status = MooseCouplingConvergenceReason::REACH_MAX_ITS;
      converged = true;
    }
    else
    {
      _coupling_status = MooseCouplingConvergenceReason::DIVERGED_MAX_ITS;
      converged = false;
    }
    return true;
  }
  return false;
}

void
IterativeMultiAppSolve::printCouplingConvergenceReason()
{
  _console << "Multiapp coupling convergence reason: ";
  switch (_coupling_status)
  {
    case MooseCouplingConvergenceReason::CONVERGED_ABS:
      _console << "CONVERGED_ABS";
      break;
    case MooseCouplingConvergenceReason::CONVERGED_RELATIVE:
      _console << "CONVERGED_RELATIVE";
      break;
    case MooseCouplingConvergenceReason::CONVERGED_CUSTOM:
      _console << "CONVERGED_CUSTOM";
      break;
    case MooseCouplingConvergenceReason::REACH_MAX_ITS:
      _console << "REACH_MAX_ITS";
      break;
    case MooseCouplingConvergenceReason::DIVERGED_MAX_ITS:
      _console << "DIVERGED_MAX_ITS";
      break;
    case MooseCouplingConvergenceReason::DIVERGED_NONLINEAR:
      _console << "DIVERGED_NONLINEAR";
      break;
    case MooseCouplingConvergenceReason::DIVERGED_FAILED_MULTIAPP:
      _console << "DIVERGED_FAILED_MULTIAPP";
      break;
    default:
      // UNSOLVED and CONVERGED_NONLINEAR should not be hit when coupling
      // iteration is not on here
      mooseError("Internal error: wrong coupling status!");
      break;
  }
  _console << std::endl;
}

bool
IterativeMultiAppSolve::autoAdvance() const
{
  bool auto_advance = !(_has_coupling_its && _problem.isTransient());

  if (dynamic_cast<EigenExecutionerBase *>(&_executioner) && _has_coupling_its)
    auto_advance = true;

  if (_auto_advance_set_by_user)
    auto_advance = _auto_advance_user_value;

  return auto_advance;
}
