//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "DefaultNonlinearConvergence.h"
#include "FEProblemBase.h"
#include "PetscSupport.h"
#include "NonlinearSystemBase.h"

#include "libmesh/equation_systems.h"

// PETSc includes
#include <petsc.h>
#include <petscmat.h>
#include <petscsnes.h>

registerMooseObject("MooseApp", DefaultNonlinearConvergence);

InputParameters
DefaultNonlinearConvergence::validParams()
{
  InputParameters params = Convergence::validParams();
  params += FEProblemSolve::feProblemDefaultConvergenceParams();

  params.addPrivateParam<bool>("added_as_default", false);

  params.addClassDescription("Default convergence criteria for FEProblem.");

  return params;
}

DefaultNonlinearConvergence::DefaultNonlinearConvergence(const InputParameters & parameters)
  : Convergence(parameters),
    _added_as_default(getParam<bool>("added_as_default")),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _nl_abs_div_tol(getSharedExecutionerParam<Real>("nl_abs_div_tol")),
    _nl_rel_div_tol(getSharedExecutionerParam<Real>("nl_div_tol")),
    _div_threshold(std::numeric_limits<Real>::max()),
    _nl_forced_its(getSharedExecutionerParam<unsigned int>("nl_forced_its")),
    _nl_max_pingpong(getSharedExecutionerParam<unsigned int>("n_max_nonlinear_pingpong")),
    _nl_current_pingpong(0)
{
  EquationSystems & es = _fe_problem.es();

  es.parameters.set<unsigned int>("nonlinear solver maximum iterations") =
      getSharedExecutionerParam<unsigned int>("nl_max_its");
  es.parameters.set<unsigned int>("nonlinear solver maximum function evaluations") =
      getSharedExecutionerParam<unsigned int>("nl_max_funcs");
  es.parameters.set<Real>("nonlinear solver absolute residual tolerance") =
      getSharedExecutionerParam<Real>("nl_abs_tol");
  es.parameters.set<Real>("nonlinear solver relative residual tolerance") =
      getSharedExecutionerParam<Real>("nl_rel_tol");
  es.parameters.set<Real>("nonlinear solver divergence tolerance") =
      getSharedExecutionerParam<Real>("nl_div_tol");
  es.parameters.set<Real>("nonlinear solver absolute step tolerance") =
      getSharedExecutionerParam<Real>("nl_abs_step_tol");
  es.parameters.set<Real>("nonlinear solver relative step tolerance") =
      getSharedExecutionerParam<Real>("nl_rel_step_tol");
}

void
DefaultNonlinearConvergence::initialSetup()
{
  Convergence::initialSetup();

  checkDuplicateSetSharedExecutionerParams();
}

bool
DefaultNonlinearConvergence::checkRelativeConvergence(const unsigned int /*it*/,
                                                      const Real fnorm,
                                                      const Real ref_norm,
                                                      const Real rel_tol,
                                                      const Real /*abs_tol*/,
                                                      std::ostringstream & oss)
{
  if (fnorm <= ref_norm * rel_tol)
  {
    oss << "Converged due to residual norm " << fnorm << " < relative tolerance (" << rel_tol
        << ")\n";
    return true;
  }
  else
    return false;
}

Convergence::MooseConvergenceStatus
DefaultNonlinearConvergence::checkConvergence(unsigned int iter)
{
  TIME_SECTION(_perfid_check_convergence);

  NonlinearSystemBase & system = _fe_problem.currentNonlinearSystem();
  MooseConvergenceStatus status = MooseConvergenceStatus::ITERATING;

  // Needed by ResidualReferenceConvergence
  nonlinearConvergenceSetup();

  SNES snes = system.getSNES();

  // ||u||
  PetscReal xnorm;
  LibmeshPetscCallA(_fe_problem.comm().get(), SNESGetSolutionNorm(snes, &xnorm));

  // ||r||
  PetscReal fnorm;
  LibmeshPetscCallA(_fe_problem.comm().get(), SNESGetFunctionNorm(snes, &fnorm));

  // ||du||
  PetscReal snorm;
  LibmeshPetscCallA(_fe_problem.comm().get(), SNESGetUpdateNorm(snes, &snorm));

  // Get current number of function evaluations done by SNES
  PetscInt nfuncs;
  LibmeshPetscCallA(_fe_problem.comm().get(), SNESGetNumberFunctionEvals(snes, &nfuncs));

  // Get tolerances from SNES
  PetscReal abs_tol, rel_tol, rel_step_tol;
  PetscInt max_its, max_funcs;
  LibmeshPetscCallA(
      _fe_problem.comm().get(),
      SNESGetTolerances(snes, &abs_tol, &rel_tol, &rel_step_tol, &max_its, &max_funcs));

#if !PETSC_VERSION_LESS_THAN(3, 8, 4)
  PetscBool force_iteration = PETSC_FALSE;
  LibmeshPetscCallA(_fe_problem.comm().get(), SNESGetForceIteration(snes, &force_iteration));

  // if PETSc says to force iteration, then force at least one iteration
  if (force_iteration && !(_nl_forced_its))
    _nl_forced_its = 1;

  // if specified here to force iteration, but PETSc doesn't know, tell it
  if (!force_iteration && (_nl_forced_its))
  {
    LibmeshPetscCallA(_fe_problem.comm().get(), SNESSetForceIteration(snes, PETSC_TRUE));
  }
#endif

  // See if SNESSetFunctionDomainError() has been called.  Note:
  // SNESSetFunctionDomainError() and SNESGetFunctionDomainError()
  // were added in different releases of PETSc.
  PetscBool domainerror;
  LibmeshPetscCallA(_fe_problem.comm().get(), SNESGetFunctionDomainError(snes, &domainerror));
  if (domainerror)
    status = MooseConvergenceStatus::DIVERGED;

  Real fnorm_old;
  // This is the first residual before any iterations have been done, but after
  // solution-modifying objects (if any) have been imposed on the solution vector.
  // We save it, and use it to detect convergence if system.usePreSMOResidual() == false.
  if (iter == 0)
  {
    system.setInitialResidual(fnorm);
    fnorm_old = fnorm;
    _nl_current_pingpong = 0;
  }
  else
    fnorm_old = system._last_nl_rnorm;

  // Check for nonlinear residual ping-pong.
  // Ping-pong will always start from a residual increase
  if ((_nl_current_pingpong % 2 == 1 && !(fnorm > fnorm_old)) ||
      (_nl_current_pingpong % 2 == 0 && fnorm > fnorm_old))
    _nl_current_pingpong += 1;
  else
    _nl_current_pingpong = 0;

  std::ostringstream oss;
  if (fnorm != fnorm)
  {
    oss << "Failed to converge, residual norm is NaN\n";
    status = MooseConvergenceStatus::DIVERGED;
  }
  else if ((iter >= _nl_forced_its) && fnorm < abs_tol)
  {
    oss << "Converged due to residual norm " << fnorm << " < " << abs_tol << '\n';
    status = MooseConvergenceStatus::CONVERGED;
  }
  else if (nfuncs >= max_funcs)
  {
    oss << "Exceeded maximum number of residual evaluations: " << nfuncs << " > " << max_funcs
        << '\n';
    status = MooseConvergenceStatus::DIVERGED;
  }
  else if ((iter >= _nl_forced_its) && iter && fnorm > system._last_nl_rnorm &&
           fnorm >= _div_threshold)
  {
    oss << "Nonlinear solve was blowing up!\n";
    status = MooseConvergenceStatus::DIVERGED;
  }
  if ((iter >= _nl_forced_its) && iter && status == MooseConvergenceStatus::ITERATING)
  {
    const auto ref_residual = system.referenceResidual();
    if (checkRelativeConvergence(iter, fnorm, ref_residual, rel_tol, abs_tol, oss))
      status = MooseConvergenceStatus::CONVERGED;
    else if (snorm < rel_step_tol * xnorm)
    {
      oss << "Converged due to small update length: " << snorm << " < " << rel_step_tol << " * "
          << xnorm << '\n';
      status = MooseConvergenceStatus::CONVERGED;
    }
    else if (_nl_rel_div_tol > 0 && fnorm > ref_residual * _nl_rel_div_tol)
    {
      oss << "Diverged due to relative residual " << ref_residual << " > divergence tolerance "
          << _nl_rel_div_tol << " * relative residual " << ref_residual << '\n';
      status = MooseConvergenceStatus::DIVERGED;
    }
    else if (_nl_abs_div_tol > 0 && fnorm > _nl_abs_div_tol)
    {
      oss << "Diverged due to residual " << fnorm << " > absolute divergence tolerance "
          << _nl_abs_div_tol << '\n';
      status = MooseConvergenceStatus::DIVERGED;
    }
    else if (_nl_current_pingpong > _nl_max_pingpong)
    {
      oss << "Diverged due to maximum nonlinear residual pingpong achieved" << '\n';
      status = MooseConvergenceStatus::DIVERGED;
    }
  }

  system._last_nl_rnorm = fnorm;
  system._current_nl_its = static_cast<unsigned int>(iter);

  std::string msg;
  msg = oss.str();
  if (_app.multiAppLevel() > 0)
    MooseUtils::indentMessage(_app.name(), msg);
  if (msg.length() > 0)
#if !PETSC_VERSION_LESS_THAN(3, 17, 0)
    LibmeshPetscCallA(_fe_problem.comm().get(), PetscInfo(snes, "%s", msg.c_str()));
#else
    LibmeshPetscCallA(_fe_problem.comm().get(), PetscInfo(snes, msg.c_str()));
#endif

  verboseOutput(oss);

  return status;
}

void
DefaultNonlinearConvergence::checkDuplicateSetSharedExecutionerParams() const
{
  if (_duplicate_shared_executioner_params.size() > 0 && !_added_as_default)
  {
    std::ostringstream oss;
    oss << "The following parameters were set in both this Convergence object and the "
           "executioner:\n";
    for (const auto & param : _duplicate_shared_executioner_params)
      oss << "  " << param << "\n";
    mooseError(oss.str());
  }
}
