//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PicardSolve.h"

#include "FEProblem.h"
#include "MooseMesh.h"
#include "NonlinearSystem.h"
#include "AllLocalDofIndicesThread.h"
#include "Console.h"

PicardSolve::PicardSolve(const InputParameters & parameters)
  : MooseObject(parameters),
    PerfGraphInterface(this),
    _problem(*getCheckedPointerParam<FEProblemBase *>(
        "_fe_problem_base", "This might happen if you don't have a mesh")),
    _nl(_problem.getNonlinearSystemBase()),
    _picard_max_its(getParam<unsigned int>("picard_max_its")),
    _has_picard_its(_picard_max_its > 1),
    _picard_rel_tol(getParam<Real>("picard_rel_tol")),
    _picard_abs_tol(getParam<Real>("picard_abs_tol")),
    _picard_force_norms(getParam<bool>("picard_force_norms")),
    _relax_factor(getParam<Real>("relaxation_factor")),
    _relaxed_vars(getParam<std::vector<std::string>>("relaxed_variables")),
    // this value will be set by MultiApp
    _picard_self_relaxation_factor(1.0),
    _max_xfem_update(getParam<unsigned int>("max_xfem_update")),
    _update_xfem_at_timestep_begin(getParam<bool>("update_xfem_at_timestep_begin")),
    _picard_timer(registerTimedSection("PicardSolve", 1)),
    _picard_it(0),
    _picard_status(MoosePicardConvergenceReason::UNSOLVED),
    _xfem_update_count(0),
    _xfem_repeat_step(false),
    _previous_entering_time(_problem.time() - 1)
{
}

bool
PicardSolve::solve()
{
  TIME_SECTION(_picard_timer);

  Real current_dt = _problem.dt();

  _picard_timestep_begin_norm.resize(_picard_max_its);
  _picard_timestep_end_norm.resize(_picard_max_its);

  bool converged = true;

  // need to back up multi-apps even when not doing Picard iteration for recovering from failed
  // multiapp solve
  _problem.backupMultiApps(EXEC_TIMESTEP_BEGIN);
  _problem.backupMultiApps(EXEC_TIMESTEP_END);

  // Prepare to relax variables as a master
  std::set<dof_id_type> relaxed_dofs;
  if (_relax_factor != 1.0)
  {
    // Snag all of the local dof indices for all of these variables
    System & libmesh_nl_system = _nl.system();
    AllLocalDofIndicesThread aldit(libmesh_nl_system, _relaxed_vars);
    ConstElemRange & elem_range = *_problem.mesh().getActiveLocalElementRange();
    Threads::parallel_reduce(elem_range, aldit);

    relaxed_dofs = aldit._all_dof_indices;
  }

  // Prepare to relax variables as a subapp
  std::set<dof_id_type> self_relaxed_dofs;
  if (_picard_self_relaxation_factor != 1.0)
  {
    // Snag all of the local dof indices for all of these variables
    System & libmesh_nl_system = _nl.system();
    AllLocalDofIndicesThread aldit(libmesh_nl_system, _picard_self_relaxed_variables);
    ConstElemRange & elem_range = *_problem.mesh().getActiveLocalElementRange();
    Threads::parallel_reduce(elem_range, aldit);

    self_relaxed_dofs = aldit._all_dof_indices;

    if (_previous_entering_time == _problem.time())
    {
      NumericVector<Number> & solution = _nl.solution();
      NumericVector<Number> & relax_previous = _nl.getVector("self_relax_previous");
      relax_previous = solution;
    }
  }

  _picard_it = 0;
  for (_picard_it = 0; _picard_it < _picard_max_its; ++_picard_it)
  {
    if (_has_picard_its)
    {
      _console << COLOR_MAGENTA << "Beginning Picard Iteration " << _picard_it << COLOR_DEFAULT
               << '\n';

      if (_picard_it == 0)
      {
        // First Picard iteration - need to save off the initial nonlinear residual
        _picard_initial_norm = _problem.computeResidualL2Norm();
        _console << COLOR_MAGENTA << "Initial Picard Norm: " << COLOR_DEFAULT;
        if (_picard_initial_norm == std::numeric_limits<Real>::max())
          _console << " MAX ";
        else
          _console << std::scientific << _picard_initial_norm;
        _console << COLOR_DEFAULT << '\n';
      }
      else
      {
        // For every iteration other than the first, we need to restore the state of the MultiApps
        _problem.restoreMultiApps(EXEC_TIMESTEP_BEGIN);
        _problem.restoreMultiApps(EXEC_TIMESTEP_END);
      }
    }

    Real begin_norm_old = (_picard_it > 0 ? _picard_timestep_begin_norm[_picard_it - 1]
                                          : std::numeric_limits<Real>::max());
    Real end_norm_old = (_picard_it > 0 ? _picard_timestep_end_norm[_picard_it - 1]
                                        : std::numeric_limits<Real>::max());
    bool relax = (_relax_factor != 1) && (_picard_it > 0);
    bool solve_converged = solveStep(begin_norm_old,
                                     _picard_timestep_begin_norm[_picard_it],
                                     end_norm_old,
                                     _picard_timestep_end_norm[_picard_it],
                                     relax,
                                     relaxed_dofs);

    if (solve_converged)
    {
      if (_has_picard_its)
      {
        _console << "\n 0 Picard |R| = "
                 << Console::outputNorm(std::numeric_limits<Real>::max(), _picard_initial_norm)
                 << '\n';

        for (unsigned int i = 1; i <= _picard_it; ++i)
        {
          Real max_norm = std::max(_picard_timestep_begin_norm[i], _picard_timestep_end_norm[i]);
          _console << std::setw(2) << i
                   << " Picard |R| = " << Console::outputNorm(_picard_initial_norm, max_norm)
                   << '\n';
        }

        Real max_norm = std::max(_picard_timestep_begin_norm[_picard_it],
                                 _picard_timestep_end_norm[_picard_it]);

        Real max_relative_drop = max_norm / _picard_initial_norm;

        if (max_norm < _picard_abs_tol)
        {
          _picard_status = MoosePicardConvergenceReason::CONVERGED_ABS;
          break;
        }
        if (max_relative_drop < _picard_rel_tol)
        {
          _picard_status = MoosePicardConvergenceReason::CONVERGED_RELATIVE;
          break;
        }
        if (_picard_it + 1 == _picard_max_its)
        {
          _picard_status = MoosePicardConvergenceReason::DIVERGED_MAX_ITS;
          converged = false;
          break;
        }
      }
    }
    else
    {
      // If the last solve didn't converge then we need to exit this step completely (even in the
      // case of Picard). So we can retry...
      converged = false;
      break;
    }

    _problem.dt() =
        current_dt; // _dt might be smaller than this at this point for multistep methods
  }

  if (converged && _picard_self_relaxation_factor != 1.0)
  {
    if (_previous_entering_time == _problem.time())
    {
      NumericVector<Number> & solution = _nl.solution();
      NumericVector<Number> & relax_previous = _nl.getVector("self_relax_previous");
      Real factor = _picard_self_relaxation_factor;
      for (const auto & dof : self_relaxed_dofs)
        solution.set(dof, (relax_previous(dof) * (1.0 - factor)) + (solution(dof) * factor));
      solution.close();
      _nl.update();
    }
    _previous_entering_time = _problem.time();
  }

  if (_has_picard_its)
  {
    _console << "Picard converged reason: ";
    switch (_picard_status)
    {
      case MoosePicardConvergenceReason::CONVERGED_ABS:
        _console << "CONVERGED_ABS";
        break;
      case MoosePicardConvergenceReason::CONVERGED_RELATIVE:
        _console << "CONVERGED_RELATIVE";
        break;
      case MoosePicardConvergenceReason::DIVERGED_MAX_ITS:
        _console << "DIVERGED_MAX_ITS";
        break;
      case MoosePicardConvergenceReason::DIVERGED_NONLINEAR:
        _console << "DIVERGED_NONLINEAR";
        break;
      case MoosePicardConvergenceReason::DIVERGED_FAILED_MULTIAPP:
        _console << "DIVERGED_FAILED_MULTIAPP";
        break;
      default:
        // UNSOLVED and CONVERGED_NONLINEAR should not be hit when Picard
        // iteration is not on here
        mooseError("Internal error: wrong Picard status!");
        break;
    }
    _console << std::endl;
  }
  return converged;
}

bool
PicardSolve::solveStep(Real begin_norm_old,
                       Real & begin_norm,
                       Real end_norm_old,
                       Real & end_norm,
                       bool relax,
                       const std::set<dof_id_type> & relaxed_dofs)
{
  _problem.execTransfers(EXEC_TIMESTEP_BEGIN);
  if (!_problem.execMultiApps(EXEC_TIMESTEP_BEGIN, !_has_picard_its))
  {
    _picard_status = MoosePicardConvergenceReason::DIVERGED_FAILED_MULTIAPP;
    return false;
  }

  if (_problem.haveXFEM() && _update_xfem_at_timestep_begin)
    _problem.updateMeshXFEM();

  _problem.execute(EXEC_TIMESTEP_BEGIN);

  if (_has_picard_its)
    if (_problem.hasMultiApps(EXEC_TIMESTEP_BEGIN) || _picard_force_norms)
    {
      begin_norm = _problem.computeResidualL2Norm();

      _console << COLOR_MAGENTA << "Picard Norm after TIMESTEP_BEGIN MultiApps: "
               << Console::outputNorm(begin_norm_old, begin_norm) << '\n';
    }

  // Perform output for timestep begin
  _problem.outputStep(EXEC_TIMESTEP_BEGIN);

  // Update warehouse active objects
  _problem.updateActiveObjects();

  if (relax)
  {
    NumericVector<Number> & solution = _nl.solution();
    NumericVector<Number> & relax_previous = _nl.getVector("relax_previous");

    // Save off the current solution
    relax_previous = solution;
  }

  _problem.solve();
  if (!_problem.converged())
  {
    _picard_status = MoosePicardConvergenceReason::DIVERGED_NONLINEAR;
    return false;
  }
  else
    _picard_status = MoosePicardConvergenceReason::CONVERGED_NONLINEAR;

  _console << COLOR_GREEN << " Solve Converged!" << COLOR_DEFAULT << std::endl;

  // Relax the "relaxed_variables"
  if (relax)
  {
    NumericVector<Number> & solution = _nl.solution();
    NumericVector<Number> & relax_previous = _nl.getVector("relax_previous");
    Real factor = _relax_factor;
    for (const auto & dof : relaxed_dofs)
      solution.set(dof, (relax_previous(dof) * (1.0 - factor)) + (solution(dof) * factor));
    solution.close();
    _nl.update();
  }

  if (_problem.haveXFEM() && (_xfem_update_count < _max_xfem_update) && _problem.updateMeshXFEM())
  {
    _console << "XFEM modifying mesh, repeating step" << std::endl;
    _xfem_repeat_step = true;
    ++_xfem_update_count;
  }
  else
  {
    if (_problem.haveXFEM())
    {
      _xfem_repeat_step = false;
      _xfem_update_count = 0;
      _console << "XFEM not modifying mesh, continuing" << std::endl;
    }

    _problem.onTimestepEnd();
    _problem.execute(EXEC_TIMESTEP_END);

    _problem.execTransfers(EXEC_TIMESTEP_END);
    if (!_problem.execMultiApps(EXEC_TIMESTEP_END, !_has_picard_its))
    {
      _picard_status = MoosePicardConvergenceReason::DIVERGED_FAILED_MULTIAPP;
      return false;
    }
  }

  if (_has_picard_its)
    if (_problem.hasMultiApps(EXEC_TIMESTEP_END) || _picard_force_norms)
    {
      end_norm = _problem.computeResidualL2Norm();

      _console << COLOR_MAGENTA << "Picard Norm after TIMESTEP_END MultiApps: "
               << Console::outputNorm(end_norm_old, end_norm) << '\n';
    }

  return true;
}
