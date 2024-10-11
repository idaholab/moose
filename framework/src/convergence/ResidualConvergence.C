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
  params += FEProblemSolve::residualConvergenceParams();

  params.addClassDescription(
      "Checks convergence based on absolute and relative error of the residual.");

  return params;
}

ResidualConvergence::ResidualConvergence(const InputParameters & parameters)
  : Convergence(parameters),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"))
{
  EquationSystems & es = _fe_problem.es();
  _l_tol = getSharedESParam<Real>("l_tol", "linear solver tolerance", es);
  _l_abs_tol = getSharedESParam<Real>("l_abs_tol", "linear solver absolute tolerance", es);
  _l_max_its = getSharedESParam<unsigned int>("l_max_its", "linear solver maximum iterations", es);
  _nl_max_its =
      getSharedESParam<unsigned int>("nl_max_its", "nonlinear solver maximum iterations", es);
  _nl_max_funcs = getSharedESParam<unsigned int>(
      "nl_max_funcs", "nonlinear solver maximum function evaluations", es);
  _nl_abs_tol =
      getSharedESParam<Real>("nl_abs_tol", "nonlinear solver absolute residual tolerance", es);
  _nl_rel_tol =
      getSharedESParam<Real>("nl_rel_tol", "nonlinear solver relative residual tolerance", es);
  _divtol = getSharedESParam<Real>("nl_div_tol", "nonlinear solver divergence tolerance", es);
  _nl_abs_step_tol =
      getSharedESParam<Real>("nl_abs_step_tol", "nonlinear solver absolute step tolerance", es);
  _nl_rel_step_tol =
      getSharedESParam<Real>("nl_rel_step_tol", "nonlinear solver relative step tolerance", es);

  if (isParamSetByUser("nl_forced_its"))
  {
    _fe_problem.setNonlinearForcedIterations(getParam<unsigned int>("nl_forced_its"));
    _nl_forced_its = getParam<unsigned int>("nl_forced_its");
  }
  else
  {
    _nl_forced_its = _fe_problem.getNonlinearForcedIterations();
  }

  if (isParamSetByUser("nl_abs_div_tol"))
    _fe_problem.setNonlinearAbsoluteDivergenceTolerance(getParam<Real>("nl_abs_div_tol"));
  else
    _nl_abs_div_tol = _fe_problem.getNonlinearAbsoluteDivergenceTolerance();

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
                                              const Real ref_norm,
                                              const Real rtol,
                                              const Real /*abstol*/,
                                              std::ostringstream & oss)
{
  if (fnorm <= ref_norm * rtol)
  {
    oss << "Converged due to function norm " << fnorm << " < relative tolerance (" << rtol << ")\n";
    return true;
  }
  else
    return false;
}

Convergence::MooseConvergenceStatus
ResidualConvergence::checkConvergence(unsigned int iter)
{
  TIME_SECTION(_perf_check_convergence);

  NonlinearSystemBase & system = _fe_problem.currentNonlinearSystem();
  MooseConvergenceStatus status = MooseConvergenceStatus::ITERATING;

  // Needed by ResidualReferenceConvergence
  nonlinearConvergenceSetup();

  SNES snes = system.getSNES();

  ierr = SNESGetSolutionNorm(snes, &xnorm_petsc);
  CHKERRABORT(_fe_problem.comm().get(), ierr);

  ierr = SNESGetFunctionNorm(snes, &fnorm_petsc);
  CHKERRABORT(_fe_problem.comm().get(), ierr);

  ierr = SNESGetUpdateNorm(snes, &snorm_petsc);
  CHKERRABORT(_fe_problem.comm().get(), ierr);

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

  if (force_iteration && !(_nl_forced_its))
    _fe_problem.setNonlinearForcedIterations(1);

  if (!force_iteration && (_nl_forced_its))
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
    status = MooseConvergenceStatus::DIVERGED;

  Real fnorm_old;

  _nl_abs_div_tol = _fe_problem.getNonlinearAbsoluteDivergenceTolerance();

  // This is the first residual before any iterations have been done,
  // but after preset BCs (if any) have been imposed on the solution
  // vector.  We save it, and use it to detect convergence if
  // compute_initial_residual_before_preset_bcs=false.
  if (iter == 0)
  {
    system.setInitialResidual(fnorm_petsc);
    // system.use_pre_SMO_residual = fnorm;
    //_initial_residual_after_preset_bcs = fnorm;
    fnorm_old = fnorm_petsc;
    _n_nl_pingpong = 0;
  }
  else
    fnorm_old = system._last_nl_rnorm;

  // Check for nonlinear residual pingpong.
  // Pingpong will always start from a residual increase
  if ((_n_nl_pingpong % 2 == 1 && !(fnorm_petsc > fnorm_old)) ||
      (_n_nl_pingpong % 2 == 0 && fnorm_petsc > fnorm_old))
    _n_nl_pingpong += 1;
  else
    _n_nl_pingpong = 0;

  long int _nl_forced_its = _fe_problem.getNonlinearForcedIterations();

  std::ostringstream oss;
  if (fnorm_petsc != fnorm_petsc)
  {
    oss << "Failed to converge, function norm is NaN\n";
    status = MooseConvergenceStatus::DIVERGED;
  }
  else if ((iter >= _nl_forced_its) && fnorm_petsc < _atol)
  {
    oss << "Converged due to function norm " << fnorm_petsc << " < " << _atol << '\n';
    status = MooseConvergenceStatus::CONVERGED;
  }
  else if (_nfuncs >= _maxf)
  {
    oss << "Exceeded maximum number of function evaluations: " << _nfuncs << " > " << _maxf << '\n';
    status = MooseConvergenceStatus::DIVERGED;
  }
  else if ((iter >= _nl_forced_its) && iter && fnorm_petsc > system._last_nl_rnorm &&
           fnorm_petsc >= _div_threshold)
  {
    oss << "Nonlinear solve was blowing up!\n";
    status = MooseConvergenceStatus::DIVERGED;
  }
  if ((iter >= _nl_forced_its) && iter && status == MooseConvergenceStatus::ITERATING)
  {
    // Set the reference residual depending on what the user asks us to use.
    const auto the_residual = system.referenceResidual();
    if (checkRelativeConvergence(iter, fnorm_petsc, the_residual, _rtol, _atol, oss))
      status = MooseConvergenceStatus::CONVERGED;
    else if (snorm_petsc < _stol * xnorm_petsc)
    {
      oss << "Converged due to small update length: " << snorm_petsc << " < " << _stol << " * "
          << xnorm_petsc << '\n';
      status = MooseConvergenceStatus::CONVERGED;
    }
    else if (_divtol > 0 && fnorm_petsc > the_residual * _divtol)
    {
      oss << "Diverged due to initial residual " << the_residual << " > divergence tolerance "
          << _divtol << " * initial residual " << the_residual << '\n';
      status = MooseConvergenceStatus::DIVERGED;
    }
    else if (_nl_abs_div_tol > 0 && fnorm_petsc > _nl_abs_div_tol)
    {
      oss << "Diverged due to residual " << fnorm_petsc << " > absolute divergence tolerance "
          << _nl_abs_div_tol << '\n';
      status = MooseConvergenceStatus::DIVERGED;
    }
    else if (_n_nl_pingpong > _n_max_nl_pingpong)
    {
      oss << "Diverged due to maximum nonlinear residual pingpong achieved" << '\n';
      status = MooseConvergenceStatus::DIVERGED;
    }
  }

  system._last_nl_rnorm = fnorm_petsc;
  system._current_nl_its = static_cast<unsigned int>(iter);

  std::string msg;
  msg = oss.str();
  if (_app.multiAppLevel() > 0)
    MooseUtils::indentMessage(_app.name(), msg);
  if (msg.length() > 0)
#if !PETSC_VERSION_LESS_THAN(3, 17, 0)
    ierr = PetscInfo(snes, "%s", msg.c_str());
#else
    ierr = PetscInfo(snes, msg.c_str());
#endif

  return status;
}
