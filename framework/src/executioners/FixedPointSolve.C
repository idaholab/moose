//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "Convergence.h"

InputParameters
FixedPointSolve::fixedPointDefaultConvergenceParams()
{
  InputParameters params = emptyInputParameters();

  params.addParam<unsigned int>(
      "fixed_point_min_its", 1, "Specifies the minimum number of fixed point iterations.");
  params.addParam<unsigned int>(
      "fixed_point_max_its", 1, "Specifies the maximum number of fixed point iterations.");
  params.addParam<bool>("disable_fixed_point_residual_norm_check",
                        false,
                        "Disable the residual norm evaluation thus the three parameters "
                        "fixed_point_rel_tol, fixed_point_abs_tol and fixed_point_force_norms.");
  params.addParam<bool>(
      "fixed_point_force_norms",
      false,
      "Force the evaluation of both the TIMESTEP_BEGIN and TIMESTEP_END norms regardless of the "
      "existence of active MultiApps with those execute_on flags, default: false.");
  params.addParam<bool>(
      "accept_on_max_fixed_point_iteration",
      false,
      "True to treat reaching the maximum number of fixed point iterations as converged.");
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

  params.addParam<PostprocessorName>("custom_pp",
                                     "Postprocessor for custom fixed point convergence check.");
  params.addParam<bool>("direct_pp_value",
                        false,
                        "True to use direct postprocessor value "
                        "(scaled by value on first iteration). "
                        "False (default) to use difference in postprocessor "
                        "value between fixed point iterations.");
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

  params.addParamNamesToGroup(
      "fixed_point_min_its fixed_point_max_its disable_fixed_point_residual_norm_check "
      "accept_on_max_fixed_point_iteration fixed_point_rel_tol fixed_point_abs_tol "
      "fixed_point_force_norms custom_pp direct_pp_value custom_abs_tol custom_rel_tol",
      "Fixed point iterations");

  return params;
}

InputParameters
FixedPointSolve::validParams()
{
  InputParameters params = emptyInputParameters();
  params += FixedPointSolve::fixedPointDefaultConvergenceParams();

  params.addParam<ConvergenceName>(
      "multiapp_fixed_point_convergence",
      "Name of the Convergence object to use to assess convergence of the "
      "MultiApp fixed point solve. If not provided, a default Convergence "
      "will be constructed internally from the executioner parameters.");

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
      "multiapp_fixed_point_convergence "
      "relaxation_factor transformed_variables transformed_postprocessors auto_advance",
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
    _has_fixed_point_its(getParam<unsigned int>("fixed_point_max_its") > 1 ||
                         isParamSetByUser("multiapp_fixed_point_convergence")),
    _relax_factor(getParam<Real>("relaxation_factor")),
    _transformed_vars(getParam<std::vector<std::string>>("transformed_variables")),
    _transformed_pps(getParam<std::vector<PostprocessorName>>("transformed_postprocessors")),
    // this value will be set by MultiApp
    _secondary_relaxation_factor(1.0),
    _fixed_point_it(0),
    _fixed_point_status(MooseFixedPointConvergenceReason::UNSOLVED),
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

  if (isParamValid("multiapp_fixed_point_convergence"))
    _problem.setMultiAppFixedPointConvergenceName(
        getParam<ConvergenceName>("multiapp_fixed_point_convergence"));
  else
    _problem.setNeedToAddDefaultMultiAppFixedPointConvergence();
}

void
FixedPointSolve::initialSetup()
{
  SolveObject::initialSetup();

  if (_has_fixed_point_its)
  {
    auto & conv = _problem.getConvergence(_problem.getMultiAppFixedPointConvergenceName());
    conv.checkIterationType(Convergence::IterationType::MULTIAPP_FIXED_POINT);
  }
}

bool
FixedPointSolve::solve()
{
  TIME_SECTION("PicardSolve", 1);

  Real current_dt = _problem.dt();

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
    libMesh::ConstElemRange & elem_range = *_problem.mesh().getActiveLocalElementRange();
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
      libMesh::ConstElemRange & elem_range = *_problem.mesh().getActiveLocalElementRange();
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

  if (_has_fixed_point_its)
  {
    auto & convergence = _problem.getConvergence(_problem.getMultiAppFixedPointConvergenceName());
    convergence.initialize();
  }

  _fixed_point_it = 0;
  while (true)
  {
    if (_has_fixed_point_its)
    {
      if (_fixed_point_it != 0)
      {
        // For every iteration other than the first, we need to restore the state of the MultiApps
        _problem.restoreMultiApps(EXEC_TIMESTEP_BEGIN);
        _problem.restoreMultiApps(EXEC_TIMESTEP_END);
      }

      _console << COLOR_MAGENTA << "Beginning fixed point iteration " << _fixed_point_it
               << COLOR_DEFAULT << std::endl
               << std::endl;
    }

    // Solve a single application for one time step
    const bool solve_converged = solveStep(transformed_dofs);

    if (solve_converged)
    {
      if (_has_fixed_point_its)
      {
        // Examine convergence metrics & properties and set the convergence reason
        bool break_out = examineFixedPointConvergence(converged);

        if (break_out)
        {
          // Except DefaultMultiAppFixedPointConvergence, convergence objects will not
          // update _fixed_point_status, so we give those cases generic values:
          if (_fixed_point_status == MooseFixedPointConvergenceReason::CONVERGED_NONLINEAR)
          {
            if (converged)
              _fixed_point_status = MooseFixedPointConvergenceReason::CONVERGED_OBJECT;
            else
              _fixed_point_status = MooseFixedPointConvergenceReason::DIVERGED_OBJECT;
          }

          break;
        }
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

    _fixed_point_it++;

    if (!_has_fixed_point_its)
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

  return converged;
}

void
FixedPointSolve::saveAllValues(const bool primary)
{
  saveVariableValues(primary);
  savePostprocessorValues(primary);
}

bool
FixedPointSolve::solveStep(const std::set<dof_id_type> & transformed_dofs)
{
  bool auto_advance = autoAdvance();

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

  if (_has_fixed_point_its)
  {
    auto & convergence = _problem.getConvergence(_problem.getMultiAppFixedPointConvergenceName());
    convergence.preExecute();
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

  return true;
}

bool
FixedPointSolve::examineFixedPointConvergence(bool & converged)
{
  _problem.execute(EXEC_MULTIAPP_FIXED_POINT_CONVERGENCE);

  auto & convergence = _problem.getConvergence(_problem.getMultiAppFixedPointConvergenceName());
  const auto status = convergence.checkConvergence(_fixed_point_it);
  switch (status)
  {
    case Convergence::MooseConvergenceStatus::CONVERGED:
      converged = true;
      return true;
    case Convergence::MooseConvergenceStatus::DIVERGED:
      converged = false;
      return true;
    case Convergence::MooseConvergenceStatus::ITERATING:
      converged = false;
      return false;
    default:
      mooseError("Should not reach here");
  }
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
    case MooseFixedPointConvergenceReason::CONVERGED_PP:
      _console << "CONVERGED_PP";
      break;
    case MooseFixedPointConvergenceReason::REACH_MAX_ITS:
      _console << "REACH_MAX_ITS";
      break;
    case MooseFixedPointConvergenceReason::CONVERGED_OBJECT:
      _console << "CONVERGED_OBJECT (see Convergence object)";
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
    case MooseFixedPointConvergenceReason::DIVERGED_OBJECT:
      _console << "DIVERGED_OBJECT (see Convergence object)";
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
