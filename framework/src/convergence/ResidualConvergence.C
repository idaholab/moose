//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ResidualConvergence.h"
#include "FEProblemBase.h"
#include "PetscSupport.h"
#include "Executioner.h"
#include "PerfGraphInterface.h"
#include "NonlinearSystemBase.h"
#include "ActionWarehouse.h"

// PETSc includes
#include <petscsnes.h>
#include <petscksp.h>
#include <petscdm.h>

// PetscDMMoose include
#include "PetscDMMoose.h"

registerMooseObject("MooseApp", ResidualConvergence);

InputParameters
ResidualConvergence::validParams()
{
  InputParameters params = Convergence::validParams();
  params += FEProblemSolve::commonParams();

  params.addClassDescription(
      "Check the convergence of residual during nonlinear iterations. Return "
      "CONVERGED if the residual is less than user-supplied stopping criteria,"
      "DIVERGED if the residual is greater than the tolerance, or ITERATING otherwise.");

  return params;
}

ResidualConvergence::ResidualConvergence(const InputParameters & parameters)
  : Convergence(parameters),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"))
{
  EquationSystems & es = _fe_problem.es();

  if (isParamSetByUser("l_tol"))
  {
    es.parameters.set<Real>("linear solver tolerance") = getParam<Real>("l_tol");
    _l_tol = getParam<Real>("l_tol");
  }
  else
  {
    _l_tol = es.parameters.get<Real>("linear solver tolerance");
  }
  if (isParamSetByUser("l_abs_tol"))
  {
    es.parameters.set<Real>("linear solver absolute tolerance") = getParam<Real>("l_abs_tol");
    _l_abs_tol = getParam<Real>("l_abs_tol");
  }
  else
  {
    _l_abs_tol = es.parameters.get<Real>("linear solver absolute tolerance");
  }

  if (isParamSetByUser("l_max_its"))
  {
    es.parameters.set<unsigned int>("linear solver maximum iterations") =
        getParam<unsigned int>("l_max_its");
    _l_max_its = getParam<unsigned int>("l_max_its");
  }
  else
  {
    _l_max_its = es.parameters.get<unsigned int>("linear solver maximum iterations");
  }

  if (isParamSetByUser("nl_max_its"))
  {
    es.parameters.set<unsigned int>("nonlinear solver maximum iterations") =
        getParam<unsigned int>("nl_max_its");
    _nl_max_its = getParam<unsigned int>("nl_max_its");
  }
  else
  {
    _nl_max_its = es.parameters.get<unsigned int>("nonlinear solver maximum iterations");
  }

  if (isParamSetByUser("nl_forced_its"))
  {
    _fe_problem.setNonlinearForcedIterations(getParam<unsigned int>("nl_forced_its"));
    _nl_forced_its = getParam<unsigned int>("nl_forced_its");
  }
  else
  {
    _nl_forced_its = _fe_problem.getNonlinearForcedIterations();
  }

  if (isParamSetByUser("nl_max_funcs"))
  {
    es.parameters.set<unsigned int>("nonlinear solver maximum function evaluations") =
        getParam<unsigned int>("nl_max_funcs");
    _nl_max_funcs = getParam<unsigned int>("nl_max_funcs");
  }
  else
  {
    _nl_max_funcs =
        es.parameters.get<unsigned int>("nonlinear solver maximum function evaluations");
  }

  if (isParamSetByUser("nl_abs_tol"))
  {
    es.parameters.set<Real>("nonlinear solver absolute residual tolerance") =
        getParam<Real>("nl_abs_tol");
    _nl_abs_tol = getParam<Real>("nl_abs_tol");
  }
  else
  {
    _nl_abs_tol = es.parameters.get<Real>("nonlinear solver absolute residual tolerance");
  }

  if (isParamSetByUser("nl_rel_tol"))
  {
    es.parameters.set<Real>("nonlinear solver relative residual tolerance") =
        getParam<Real>("nl_rel_tol");
    _nl_rel_tol = getParam<Real>("nl_rel_tol");
  }
  else
  {
    _nl_rel_tol = es.parameters.get<Real>("nonlinear solver relative residual tolerance");
  }

  if (isParamSetByUser("nl_div_tol"))
  {
    es.parameters.set<Real>("nonlinear solver divergence tolerance") = getParam<Real>("nl_div_tol");
    _divtol = getParam<Real>("nl_div_tol");
  }
  else
  {
    _divtol = es.parameters.get<Real>("nonlinear solver divergence tolerance");
  }

  if (isParamSetByUser("nl_abs_div_tol"))
    _fe_problem.setNonlinearAbsoluteDivergenceTolerance(getParam<Real>("nl_abs_div_tol"));
  else
    _nl_abs_div_tol = _fe_problem.getNonlinearAbsoluteDivergenceTolerance();

  if (isParamSetByUser("nl_abs_step_tol"))
  {
    es.parameters.set<Real>("nonlinear solver absolute step tolerance") =
        getParam<Real>("nl_abs_step_tol");
    _nl_abs_step_tol = getParam<Real>("nl_abs_step_tol");
  }
  else
  {
    _nl_abs_step_tol = es.parameters.get<Real>("nonlinear solver absolute step tolerance");
  }

  if (isParamSetByUser("nl_rel_step_tol"))
  {
    es.parameters.set<Real>("nonlinear solver relative step tolerance") =
        getParam<Real>("nl_rel_step_tol");
    _nl_rel_step_tol = getParam<Real>("nl_rel_step_tol");
  }
  else
  {
    _nl_rel_step_tol = es.parameters.get<Real>("nonlinear solver relative step tolerance");
  }

  if (isParamSetByUser("nl_max_nonlinear_pingpong"))
  {
    _fe_problem.setMaxNLPingPong(getParam<unsigned int>("n_max_nonlinear_pingpong"));
    _n_max_nl_pingpong = getParam<unsigned int>("n_max_nonlinear_pingpong");
  }
  else
  {
    _n_max_nl_pingpong = _fe_problem.getMaxNLPingPong();
  }
}

bool
ResidualConvergence::checkRelativeConvergence(const PetscInt /*it*/,
                                              const Real fnorm,
                                              const Real the_residual,
                                              const Real rtol,
                                              const Real /*abstol*/,
                                              std::ostringstream & oss)
{
  if (_fe_problem.getFailNextNonlinearConvergenceCheck())
    return false;
  if (fnorm <= the_residual * rtol)
  {
    oss << "Converged due to function norm " << fnorm << " < relative tolerance (" << rtol << ")\n";
    return true;
  }
  return false;
}

Convergence::MooseAlgebraicConvergence
ResidualConvergence::checkAlgebraicConvergence(int it, Real xnorm, Real snorm, Real fnorm)
{
  TIME_SECTION(_perf_nonlinear);

  NonlinearSystemBase & system = _fe_problem.currentNonlinearSystem();
  MooseAlgebraicConvergence reason = MooseAlgebraicConvergence::ITERATING;

  // Needed by ResidualReferenceConvergence
  nonlinearConvergenceSetup();

  // To check if the nonlinear iterations should abort
  if (_fe_problem.getFailNextNonlinearConvergenceCheck())
  {
    _fe_problem.resetFailNextNonlinearConvergenceCheck();
    return MooseAlgebraicConvergence::DIVERGED;
  }

  // To check PETSc error codes.
  PetscErrorCode ierr = 0;
  SNES snes = system.getSNES();

  // Ask the SNES object about its tolerances.
  ierr = SNESGetTolerances(snes, &_atol, &_rtol, &_stol, &_maxit, &_maxf);
  CHKERRABORT(_fe_problem.comm().get(), ierr);

  // Ask the SNES object about its divergence tolerance
#if !PETSC_VERSION_LESS_THAN(3, 8, 0)
  ierr = SNESGetDivergenceTolerance(snes, &_divtol);
  CHKERRABORT(_fe_problem.comm().get(), ierr);
#endif

  // Get current number of function evaluations done by SNES.
  ierr = SNESGetNumberFunctionEvals(snes, &_nfuncs);
  CHKERRABORT(_fe_problem.comm().get(), ierr);

  // Whether or not to force SNESSolve() take at least one iteration regardless of the initial
  // residual norm
#if !PETSC_VERSION_LESS_THAN(3, 8, 4)
  PetscBool force_iteration = PETSC_FALSE;
  ierr = SNESGetForceIteration(snes, &force_iteration);
  CHKERRABORT(_fe_problem.comm().get(), ierr);

  if (force_iteration && !(_fe_problem.getNonlinearForcedIterations()))
    _fe_problem.setNonlinearForcedIterations(1);

  if (!force_iteration && (_fe_problem.getNonlinearForcedIterations()))
  {
    ierr = SNESSetForceIteration(snes, PETSC_TRUE);
    CHKERRABORT(_fe_problem.comm().get(), ierr);
  }
#endif

  // See if SNESSetFunctionDomainError() has been called.  Note:
  // SNESSetFunctionDomainError() and SNESGetFunctionDomainError()
  // were added in different releases of PETSc.
  PetscBool domainerror;
  ierr = SNESGetFunctionDomainError(snes, &domainerror);
  CHKERRABORT(_fe_problem.comm().get(), ierr);
  if (domainerror)
  {
    reason = MooseAlgebraicConvergence::DIVERGED;
  }

  Real fnorm_old;

  _nl_abs_div_tol = _fe_problem.getNonlinearAbsoluteDivergenceTolerance();

  // This is the first residual before any iterations have been done,
  // but after preset BCs (if any) have been imposed on the solution
  // vector.  We save it, and use it to detect convergence if
  // compute_initial_residual_before_preset_bcs=false.
  if (it == 0)
  {
    system.setInitialResidual(fnorm);
    // system.use_pre_SMO_residual = fnorm;
    //_initial_residual_after_preset_bcs = fnorm;
    fnorm_old = fnorm;
    _n_nl_pingpong = 0;
  }
  else
    fnorm_old = system._last_nl_rnorm;

  // Check for nonlinear residual pingpong.
  // Pingpong will always start from a residual increase
  if ((_n_nl_pingpong % 2 == 1 && !(fnorm > fnorm_old)) ||
      (_n_nl_pingpong % 2 == 0 && fnorm > fnorm_old))
    _n_nl_pingpong += 1;
  else
    _n_nl_pingpong = 0;

  long int _nl_forced_its = _fe_problem.getNonlinearForcedIterations();

  std::ostringstream oss;
  if (fnorm != fnorm)
  {
    oss << "Failed to converge, function norm is NaN\n";
    reason = MooseAlgebraicConvergence::DIVERGED;
  }
  else if ((it >= _nl_forced_its) && fnorm < _atol)
  {
    oss << "Converged due to function norm " << fnorm << " < " << _atol << '\n';
    reason = MooseAlgebraicConvergence::CONVERGED;
  }
  else if (_nfuncs >= _maxf)
  {
    oss << "Exceeded maximum number of function evaluations: " << _nfuncs << " > " << _maxf << '\n';
    reason = MooseAlgebraicConvergence::DIVERGED;
  }
  else if ((it >= _nl_forced_its) && it && fnorm > system._last_nl_rnorm && fnorm >= _div_threshold)
  {
    oss << "Nonlinear solve was blowing up!\n";
    reason = MooseAlgebraicConvergence::DIVERGED;
  }
  if ((it >= _nl_forced_its) && it && reason == MooseAlgebraicConvergence::ITERATING)
  {
    // Set the reference residual depending on what the user asks us to use.
    const auto the_residual = system.referenceResidual();
    if (checkRelativeConvergence(it, fnorm, the_residual, _rtol, _atol, oss))
      reason = MooseAlgebraicConvergence::CONVERGED;
    else if (snorm < _stol * xnorm)
    {
      oss << "Converged due to small update length: " << snorm << " < " << _stol << " * " << xnorm
          << '\n';
      reason = MooseAlgebraicConvergence::CONVERGED;
    }
    else if (_divtol > 0 && fnorm > the_residual * _divtol)
    {
      oss << "Diverged due to initial residual " << the_residual << " > divergence tolerance "
          << _divtol << " * initial residual " << the_residual << '\n';
      reason = MooseAlgebraicConvergence::DIVERGED;
    }
    else if (_nl_abs_div_tol > 0 && fnorm > _nl_abs_div_tol)
    {
      oss << "Diverged due to residual " << fnorm << " > absolute divergence tolerance "
          << _nl_abs_div_tol << '\n';
      reason = MooseAlgebraicConvergence::DIVERGED;
    }
    else if (_n_nl_pingpong > _n_max_nl_pingpong)
    {
      oss << "Diverged due to maximum nonlinear residual pingpong achieved" << '\n';
      reason = MooseAlgebraicConvergence::DIVERGED;
    }
  }

  system._last_nl_rnorm = fnorm;
  system._current_nl_its = static_cast<unsigned int>(it);

  std::string msg;
  msg = oss.str();
  if (_app.multiAppLevel() > 0)
    MooseUtils::indentMessage(_app.name(), msg);
  if (msg.length() > 0)
#if !PETSC_VERSION_LESS_THAN(3, 17, 0)
    PetscInfo(snes, "%s", msg.c_str());
#else
    PetscInfo(snes, msg.c_str());
#endif

  return reason;
}
