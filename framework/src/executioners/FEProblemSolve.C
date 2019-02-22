//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FEProblemSolve.h"

#include "FEProblem.h"
#include "EigenProblem.h"
#include "ExternalProblem.h"
#include "Executioner.h"
#include "MooseMesh.h"
#include "NonlinearSystem.h"
#include "AllLocalDofIndicesThread.h"
#include "Console.h"

#include "libmesh/parallel.h"
#include "libmesh/parallel_implementation.h"

template <>
InputParameters
validParams<FEProblemSolve>()
{
  InputParameters params = emptyInputParameters();

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
  params.addParam<bool>("compute_initial_residual_before_preset_bcs",
                        false,
                        "Use the residual norm computed *before* PresetBCs are imposed in relative "
                        "convergence check");

  params.addParamNamesToGroup("l_tol l_abs_step_tol l_max_its nl_max_its nl_max_funcs "
                              "nl_abs_tol nl_rel_tol nl_abs_step_tol nl_rel_step_tol "
                              "snesmf_reuse_base compute_initial_residual_before_preset_bcs",
                              "Solver");

  params.addParam<bool>("no_feproblem_solve", false, "");
  return params;
}

FEProblemSolve::FEProblemSolve(Executioner * ex)
  : SolveObject(ex),
    _splitting(getParam<std::vector<std::string>>("splitting")),
    _no_feproblem_solve(getParam<bool>("no_feproblem_solve")),
    _solve_timer(registerTimedSection("solve", 1)),
    _check_nonlinear_convergence_timer(registerTimedSection("checkNonlinearConvergence", 5)),
    _check_linear_convergence_timer(registerTimedSection("checkLinearConvergence", 5)),
    _check_exception_and_stop_solve_timer(registerTimedSection("checkExceptionAndStopSolve", 5)),
    _fail_next_linear_convergence_check(false)
{
  if (_pars.isParamSetByUser("line_search"))
    _problem.addLineSearch(_pars);

// Extract and store PETSc related settings on FEProblemBase
#ifdef LIBMESH_HAVE_PETSC
  Moose::PetscSupport::storePetscOptions(_problem, _pars);
#endif // LIBMESH_HAVE_PETSC

  EquationSystems & es = _problem.es();
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

  _nl._compute_initial_residual_before_preset_bcs =
      getParam<bool>("compute_initial_residual_before_preset_bcs");

  _nl._l_abs_step_tol = getParam<Real>("l_abs_step_tol");

  _problem.setSNESMFReuseBase(getParam<bool>("snesmf_reuse_base"),
                              _pars.isParamSetByUser("snesmf_reuse_base"));
}

bool
FEProblemSolve::solve()
{
  if (_no_feproblem_solve)
    return true;

  TIME_SECTION(_solve_timer);

  if (_inner_solve)
    mooseError("Inner solve object cannot be set for FEProblemSolve");

  auto external_problem = dynamic_cast<ExternalProblem *>(&_problem);
  if (external_problem)
  {
    external_problem->solve();
    return external_problem->converged();
  }

  auto eigen_problem = dynamic_cast<EigenProblem *>(&_problem);

  if (eigen_problem)
  {
#if LIBMESH_HAVE_SLEPC
    // Make sure the SLEPc options are setup for this app
    Moose::SlepcSupport::slepcSetOptions(*eigen_problem, _pars);
#endif
  }
  else
  {
#ifdef LIBMESH_HAVE_PETSC
    Moose::PetscSupport::petscSetOptions(
        _problem); // Make sure the PETSc options are setup for this app
#endif

    Moose::setSolverDefaults(_problem);

    // Setup the output system for printing linear/nonlinear iteration information
    _executioner.initPetscOutput();

    _problem.possiblyRebuildGeomSearchPatches();

    // reset flag so that linear solver does not use
    // the old converged reason "DIVERGED_NANORINF", when
    // we throw  an exception and stop solve
    _fail_next_linear_convergence_check = false;
  }

  if (_problem.needSolve())
  {
    _nl.solve();
    _nl.update();
  }

  // sync solutions in displaced problem
  if (_displaced_problem)
    _displaced_problem->syncSolutions();

  if (_problem.needSolve())
    return _nl.converged() && !_executioner.augmentedFEProblemSolveFail();
  else
    return true;
}

void
FEProblemSolve::checkExceptionAndStopSolve()
{
  TIME_SECTION(_check_exception_and_stop_solve_timer);

  // See if any processor had an exception.  If it did, get back the
  // processor that the exception occurred on.
  unsigned int processor_id;

  bool has_exception = _executioner.hasException();
  _communicator.maxloc(has_exception, processor_id);

  auto exception_message = _executioner.exceptionMessage();

  if (has_exception)
  {
    _communicator.broadcast(exception_message, processor_id);

    // Print the message
    if (_communicator.rank() == 0)
      Moose::err << exception_message << std::endl;

    // Stop the solve -- this entails setting
    // SNESSetFunctionDomainError() or directly inserting NaNs in the
    // residual vector to let PETSc >= 3.6 return DIVERGED_NANORINF.
    _nl.stopSolve();

    // and close Aux system (we MUST do this here; see #11525)
    _aux.solution().close();

    // We've handled this exception, so we no longer have one.
    _executioner.clearException();

    // Force the next linear convergence check to fail.
    _fail_next_linear_convergence_check = true;

    // Repropagate the exception, so it can be caught at a higher level, typically
    // this is NonlinearSystem::computeResidual().
    throw MooseException(exception_message);
  }
}

unsigned int
FEProblemSolve::nNonlinearIterations() const
{
  return _nl.nNonlinearIterations();
}

unsigned int
FEProblemSolve::nLinearIterations() const
{
  return _nl.nLinearIterations();
}

Real
FEProblemSolve::finalNonlinearResidual() const
{
  return _nl.finalNonlinearResidual();
}

MooseLinearConvergenceReason
FEProblemSolve::checkLinearConvergence(std::string & /*msg*/,
                                       const PetscInt n,
                                       const Real rnorm,
                                       const Real /*rtol*/,
                                       const Real /*atol*/,
                                       const Real /*dtol*/,
                                       const PetscInt maxits)
{
  TIME_SECTION(_check_linear_convergence_timer);

  if (_fail_next_linear_convergence_check)
  {
    // Unset the flag
    _fail_next_linear_convergence_check = false;
    return MooseLinearConvergenceReason::DIVERGED_NANORINF;
  }

  // We initialize the reason to something that basically means MOOSE
  // has not made a decision on convergence yet.
  MooseLinearConvergenceReason reason = MooseLinearConvergenceReason::ITERATING;

  // Get a reference to our Nonlinear System
  NonlinearSystemBase & system = _problem.getNonlinearSystemBase();

  // If it's the beginning of a new set of iterations, reset
  // last_rnorm, otherwise record the most recent linear residual norm
  // in the NonlinearSystem.
  if (n == 0)
    system._last_rnorm = 1e99;
  else
    system._last_rnorm = rnorm;

  // If the linear residual norm is less than the System's linear absolute
  // step tolerance, we consider it to be converged and set the reason as
  // MooseLinearConvergenceReason::CONVERGED_RTOL.
  if (std::abs(rnorm - system._last_rnorm) < system._l_abs_step_tol)
    reason = MooseLinearConvergenceReason::CONVERGED_RTOL;

  // If we hit max its, then we consider that converged (rather than
  // KSP_DIVERGED_ITS).
  if (n >= maxits)
    reason = MooseLinearConvergenceReason::CONVERGED_ITS;

  // If either of our convergence criteria is met, store the number of linear
  // iterations in the System.
  if (reason == MooseLinearConvergenceReason::CONVERGED_ITS ||
      reason == MooseLinearConvergenceReason::CONVERGED_RTOL)
    system._current_l_its.push_back(static_cast<unsigned int>(n));

  return reason;
}

MooseNonlinearConvergenceReason
FEProblemSolve::checkNonlinearConvergence(std::string & msg,
                                          const PetscInt it,
                                          const Real xnorm,
                                          const Real snorm,
                                          const Real fnorm,
                                          const Real rtol,
                                          const Real stol,
                                          const Real abstol,
                                          const PetscInt nfuncs,
                                          const PetscInt max_funcs,
                                          const PetscBool force_iteration,
                                          const Real initial_residual_before_preset_bcs,
                                          const Real div_threshold)
{
  TIME_SECTION(_check_nonlinear_convergence_timer);

  NonlinearSystemBase & system = _problem.getNonlinearSystemBase();
  MooseNonlinearConvergenceReason reason = MooseNonlinearConvergenceReason::ITERATING;

  // This is the first residual before any iterations have been done,
  // but after PresetBCs (if any) have been imposed on the solution
  // vector.  We save it, and use it to detect convergence if
  // compute_initial_residual_before_preset_bcs=false.
  if (it == 0)
    system._initial_residual_after_preset_bcs = fnorm;

  std::ostringstream oss;
  if (fnorm != fnorm)
  {
    oss << "Failed to converge, function norm is NaN\n";
    reason = MooseNonlinearConvergenceReason::DIVERGED_FNORM_NAN;
  }
  else if (fnorm < abstol && (it || !force_iteration))
  {
    oss << "Converged due to function norm " << fnorm << " < " << abstol << '\n';
    reason = MooseNonlinearConvergenceReason::CONVERGED_FNORM_ABS;
  }
  else if (nfuncs >= max_funcs)
  {
    oss << "Exceeded maximum number of function evaluations: " << nfuncs << " > " << max_funcs
        << '\n';
    reason = MooseNonlinearConvergenceReason::DIVERGED_FUNCTION_COUNT;
  }
  else if (it && fnorm > system._last_nl_rnorm && fnorm >= div_threshold)
  {
    oss << "Nonlinear solve was blowing up!\n";
    reason = MooseNonlinearConvergenceReason::DIVERGED_LINE_SEARCH;
  }

  if (it && reason == MooseNonlinearConvergenceReason::ITERATING)
  {
    // If compute_initial_residual_before_preset_bcs==false, then use the
    // first residual computed by Petsc to determine convergence.
    Real the_residual = system._compute_initial_residual_before_preset_bcs
                            ? initial_residual_before_preset_bcs
                            : system._initial_residual_after_preset_bcs;
    if (fnorm <= the_residual * rtol)
    {
      oss << "Converged due to function norm " << fnorm << " < "
          << " (relative tolerance)\n";
      reason = MooseNonlinearConvergenceReason::CONVERGED_FNORM_RELATIVE;
    }
    else if (snorm < stol * xnorm)
    {
      oss << "Converged due to small update length: " << snorm << " < " << stol << " * " << xnorm
          << '\n';
      reason = MooseNonlinearConvergenceReason::CONVERGED_SNORM_RELATIVE;
    }
  }

  system._last_nl_rnorm = fnorm;
  system._current_nl_its = static_cast<unsigned int>(it);

  msg = oss.str();
  if (_app.multiAppLevel() > 0)
    MooseUtils::indentMessage(_app.name(), msg);

  return reason;
}
