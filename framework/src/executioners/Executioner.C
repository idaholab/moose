//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Moose includes
#include "Executioner.h"

#include "FEProblem.h"
#include "MooseApp.h"
#include "MooseMesh.h"
#include "NonlinearSystem.h"
#include "SlepcSupport.h"
#include "AllLocalDofIndicesThread.h"
#include "Console.h"

// C++ includes
#include <vector>
#include <limits>

template <>
InputParameters
validParams<Executioner>()
{
  InputParameters params = validParams<MooseObject>();
  params.addDeprecatedParam<FileNameNoExtension>(
      "restart_file_base",
      "",
      "File base name used for restart",
      "Please use \"Problem/restart_file_base\" instead");

  params.registerBase("Executioner");

  params.addParamNamesToGroup("restart_file_base", "Restart");

  params.addParam<std::vector<std::string>>("splitting",
                                            "Top-level splitting defining a "
                                            "hierarchical decomposition into "
                                            "subsystems to help the solver.");

  std::set<std::string> line_searches = {"contact", "default", "none", "basic"};
#ifdef LIBMESH_HAVE_PETSC
  std::set<std::string> petsc_line_searches = Moose::PetscSupport::getPetscValidLineSearches();
  line_searches.insert(petsc_line_searches.begin(), petsc_line_searches.end());
#endif // LIBMESH_HAVE_PETSC
  std::string line_search_string = Moose::stringify(line_searches, " ");
  MooseEnum line_search(line_search_string, "default");
  std::string addtl_doc_str(" (Note: none = basic)");
  params.addParam<MooseEnum>(
      "line_search", line_search, "Specifies the line search type" + addtl_doc_str);
  MooseEnum line_search_package("petsc moose", "petsc");
  params.addParam<MooseEnum>("line_search_package",
                             line_search_package,
                             "The solver package to use to conduct the line-search");
  params.addParam<unsigned>("contact_line_search_allowed_lambda_cuts",
                            2,
                            "The number of times lambda is allowed to be cut in half in the "
                            "contact line search. We recommend this number be roughly bounded by 0 "
                            "<= allowed_lambda_cuts <= 3");
  params.addParam<Real>("contact_line_search_ltol",
                        "The linear relative tolerance to be used while the contact state is "
                        "changing between non-linear iterations. We recommend that this tolerance "
                        "be looser than the standard linear tolerance");

// Default Solver Behavior
#ifdef LIBMESH_HAVE_PETSC
  params += Moose::PetscSupport::getPetscValidParams();
#endif // LIBMESH_HAVE_PETSC
  params.addParam<Real>("l_tol", 1.0e-5, "Linear Tolerance");
  params.addParam<Real>("l_abs_step_tol", -1, "Linear Absolute Step Tolerance");
  params.addParam<unsigned int>("l_max_its", 10000, "Max Linear Iterations");
  params.addParam<unsigned int>("nl_max_its", 50, "Max Nonlinear Iterations");
  params.addParam<unsigned int>("nl_max_funcs", 10000, "Max Nonlinear solver function evaluations");
  params.addParam<Real>("nl_abs_tol", 1.0e-50, "Nonlinear Absolute Tolerance");
  params.addParam<Real>("nl_rel_tol", 1.0e-8, "Nonlinear Relative Tolerance");
  params.addParam<Real>("nl_abs_step_tol", 1.0e-50, "Nonlinear Absolute step Tolerance");
  params.addParam<Real>("nl_rel_step_tol", 1.0e-50, "Nonlinear Relative step Tolerance");
  params.addParam<bool>(
      "snesmf_reuse_base",
      true,
      "Specifies whether or not to reuse the base vector for matrix-free calculation");
  params.addParam<bool>("no_fe_reinit", false, "Specifies whether or not to reinitialize FEs");
  params.addParam<bool>("compute_initial_residual_before_preset_bcs",
                        false,
                        "Use the residual norm computed *before* PresetBCs are imposed in relative "
                        "convergence check");

  params.addParam<unsigned int>(
      "picard_max_its",
      1,
      "Maximum number of times each timestep will be solved.  Mainly used when "
      "wanting to do Picard iterations with MultiApps that are set to "
      "execute_on timestep_end or timestep_begin. Setting this parameter to 1 turns off the Picard "
      "iterations.");
  params.addParam<Real>("picard_rel_tol",
                        1e-8,
                        "The relative nonlinear residual drop to shoot for "
                        "during Picard iterations.  This check is "
                        "performed based on the Master app's nonlinear "
                        "residual.");
  params.addParam<Real>("picard_abs_tol",
                        1e-50,
                        "The absolute nonlinear residual to shoot for "
                        "during Picard iterations.  This check is "
                        "performed based on the Master app's nonlinear "
                        "residual.");
  params.addParam<bool>(
      "picard_force_norms",
      false,
      "Force the evaluation of both the TIMESTEP_BEGIN and TIMESTEP_END norms regardless of the "
      "existance of active MultiApps with those execute_on flags, default: false.");

  params.addRangeCheckedParam<Real>("relaxation_factor",
                                    1.0,
                                    "relaxation_factor>0 & relaxation_factor<2",
                                    "Fraction of newly computed value to keep."
                                    "Set between 0 and 2.");
  params.addParam<std::vector<std::string>>("relaxed_variables",
                                            std::vector<std::string>(),
                                            "List of variables to relax during Picard Iteration");

  params.addParamNamesToGroup("picard_max_its picard_rel_tol picard_abs_tol picard_force_norms "
                              "relaxation_factor relaxed_variables",
                              "Picard");

  params.addParam<unsigned int>(
      "max_xfem_update",
      std::numeric_limits<unsigned int>::max(),
      "Maximum number of times to update XFEM crack topology in a step due to evolving cracks");
  params.addParam<bool>("update_xfem_at_timestep_begin",
                        false,
                        "Should XFEM update the mesh at the beginning of the timestep");

  params.addParamNamesToGroup("l_tol l_abs_step_tol l_max_its nl_max_its nl_max_funcs "
                              "nl_abs_tol nl_rel_tol nl_abs_step_tol nl_rel_step_tol "
                              "compute_initial_residual_before_preset_bcs",
                              "Solver");
  params.addParamNamesToGroup("no_fe_reinit", "Advanced");

  return params;
}

Executioner::Executioner(const InputParameters & parameters)
  : MooseObject(parameters),
    UserObjectInterface(this),
    PostprocessorInterface(this),
    Restartable(this, "Executioners"),
    PerfGraphInterface(this),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>(
        "_fe_problem_base", "This might happen if you don't have a mesh")),
    _picard_solve(parameters),
    _initial_residual_norm(std::numeric_limits<Real>::max()),
    _old_initial_residual_norm(std::numeric_limits<Real>::max()),
    _restart_file_base(getParam<FileNameNoExtension>("restart_file_base")),
    _splitting(getParam<std::vector<std::string>>("splitting"))
{
  if (_pars.isParamSetByUser("line_search"))
    _fe_problem.addLineSearch(_pars);

// Extract and store PETSc related settings on FEProblemBase
#ifdef LIBMESH_HAVE_PETSC
  Moose::PetscSupport::storePetscOptions(_fe_problem, _pars);
#endif // LIBMESH_HAVE_PETSC

  // solver params
  EquationSystems & es = _fe_problem.es();
  es.parameters.set<Real>("linear solver tolerance") = getParam<Real>("l_tol");

  es.parameters.set<Real>("linear solver absolute step tolerance") =
      getParam<Real>("l_abs_step_tol");

  es.parameters.set<unsigned int>("linear solver maximum iterations") =
      getParam<unsigned int>("l_max_its");

  es.parameters.set<unsigned int>("nonlinear solver maximum iterations") =
      getParam<unsigned int>("nl_max_its");

  es.parameters.set<unsigned int>("nonlinear solver maximum function evaluations") =
      getParam<unsigned int>("nl_max_funcs");

  es.parameters.set<Real>("nonlinear solver absolute residual tolerance") =
      getParam<Real>("nl_abs_tol");

  es.parameters.set<Real>("nonlinear solver relative residual tolerance") =
      getParam<Real>("nl_rel_tol");

  es.parameters.set<Real>("nonlinear solver absolute step tolerance") =
      getParam<Real>("nl_abs_step_tol");

  es.parameters.set<Real>("nonlinear solver relative step tolerance") =
      getParam<Real>("nl_rel_step_tol");

  _fe_problem.getNonlinearSystemBase()._compute_initial_residual_before_preset_bcs =
      getParam<bool>("compute_initial_residual_before_preset_bcs");

  _fe_problem.getNonlinearSystemBase()._l_abs_step_tol = getParam<Real>("l_abs_step_tol");

  _fe_problem.setSNESMFReuseBase(getParam<bool>("snesmf_reuse_base"),
                                 parameters.isParamSetByUser("snesmf_reuse_base"));

  if (getParam<Real>("relaxation_factor") != 1.0)
    // Store a copy of the previous solution here
    _fe_problem.getNonlinearSystemBase().addVector("relax_previous", false, PARALLEL);
}

Executioner::~Executioner() {}

void
Executioner::init()
{
}

void
Executioner::preExecute()
{
}

void
Executioner::postExecute()
{
}

void
Executioner::preSolve()
{
}

void
Executioner::postSolve()
{
}

Problem &
Executioner::problem()
{
  mooseDoOnce(mooseWarning("This method is deprecated, use feProblem() instead"));
  return _fe_problem;
}

FEProblemBase &
Executioner::feProblem()
{
  return _fe_problem;
}

std::string
Executioner::getTimeStepperName()
{
  return std::string();
}

bool
Executioner::lastSolveConverged()
{
  return _fe_problem.converged();
}

void
Executioner::addAttributeReporter(const std::string & name,
                                  Real & attribute,
                                  const std::string execute_on)
{
  FEProblemBase * problem = getCheckedPointerParam<FEProblemBase *>(
      "_fe_problem_base",
      "Failed to retrieve FEProblemBase when adding a attribute reporter in Executioner");
  InputParameters params = _app.getFactory().getValidParams("ExecutionerAttributeReporter");
  params.set<Real *>("value") = &attribute;
  if (!execute_on.empty())
    params.set<ExecFlagEnum>("execute_on") = execute_on;
  problem->addPostprocessor("ExecutionerAttributeReporter", name, params);
}

Executioner::PicardSolve::PicardSolve(const InputParameters & parameters)
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
Executioner::PicardSolve::solve()
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
Executioner::PicardSolve::solveStep(Real begin_norm_old,
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
