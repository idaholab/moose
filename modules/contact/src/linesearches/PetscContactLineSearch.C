//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PetscContactLineSearch.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"
#include "libmesh/petsc_nonlinear_solver.h"
#include "libmesh/petsc_solver_exception.h"
#include <petscdm.h>

registerMooseObject("ContactApp", PetscContactLineSearch);

InputParameters
PetscContactLineSearch::validParams()
{
  return ContactLineSearchBase::validParams();
}

PetscContactLineSearch::PetscContactLineSearch(const InputParameters & parameters)
  : ContactLineSearchBase(parameters)
{
  _solver = dynamic_cast<PetscNonlinearSolver<Real> *>(
      _fe_problem.getNonlinearSystem().nonlinearSolver());
  if (!_solver)
    mooseError(
        "This line search operates only with Petsc, so Petsc must be your nonlinear solver.");
}

void
PetscContactLineSearch::lineSearch()
{
  PetscBool changed_y = PETSC_FALSE, changed_w = PETSC_FALSE;
  PetscErrorCode ierr;
  Vec X, F, Y, W, G, W1;
  SNESLineSearch line_search;
  PetscReal fnorm, xnorm, ynorm, gnorm;
  PetscBool domainerror;
  PetscReal ksp_rtol, ksp_abstol, ksp_dtol;
  PetscInt ksp_maxits;
  KSP ksp;
  SNES snes = _solver->snes();

  ierr = SNESGetLineSearch(snes, &line_search);
  LIBMESH_CHKERR(ierr);
  ierr = SNESLineSearchGetVecs(line_search, &X, &F, &Y, &W, &G);
  LIBMESH_CHKERR(ierr);
  ierr = SNESLineSearchGetNorms(line_search, &xnorm, &fnorm, &ynorm);
  LIBMESH_CHKERR(ierr);
  ierr = SNESLineSearchGetSNES(line_search, &snes);
  LIBMESH_CHKERR(ierr);
  ierr = SNESLineSearchSetReason(line_search, SNES_LINESEARCH_SUCCEEDED);
  LIBMESH_CHKERR(ierr);
  ierr = SNESGetKSP(snes, &ksp);
  LIBMESH_CHKERR(ierr);
  ierr = KSPGetTolerances(ksp, &ksp_rtol, &ksp_abstol, &ksp_dtol, &ksp_maxits);
  LIBMESH_CHKERR(ierr);
  ierr = VecDuplicate(W, &W1);
  LIBMESH_CHKERR(ierr);

  if (!_user_ksp_rtol_set)
  {
    _user_ksp_rtol = ksp_rtol;
    _user_ksp_rtol_set = true;
  }

  ++_nl_its;

  /* precheck */
  ierr = SNESLineSearchPreCheck(line_search, X, Y, &changed_y);
  LIBMESH_CHKERR(ierr);

  /* temporary update */
  _contact_lambda = 1.;
  ierr = VecWAXPY(W, -_contact_lambda, Y, X);
  LIBMESH_CHKERR(ierr);

  /* compute residual to determine whether contact state has changed since the last non-linear
   * residual evaluation */
  _current_contact_state.clear();
  ierr = SNESComputeFunction(snes, W, F);
  LIBMESH_CHKERR(ierr);
  ierr = SNESGetFunctionDomainError(snes, &domainerror);
  LIBMESH_CHKERR(ierr);
  if (domainerror)
  {
    ierr = SNESLineSearchSetReason(line_search, SNES_LINESEARCH_FAILED_DOMAIN);
    LIBMESH_CHKERR(ierr);
  }
  ierr = VecNorm(F, NORM_2, &fnorm);
  LIBMESH_CHKERR(ierr);
  std::set<dof_id_type> contact_state_stored = _current_contact_state;
  _current_contact_state.clear();
  printContactInfo(contact_state_stored);

  if (_affect_ltol)
  {
    if (contact_state_stored != _old_contact_state)
    {
      KSPSetTolerances(ksp, _contact_ltol, ksp_abstol, ksp_dtol, ksp_maxits);
      _console << "Contact set changed since previous non-linear iteration!" << std::endl;
    }
    else
      KSPSetTolerances(ksp, _user_ksp_rtol, ksp_abstol, ksp_dtol, ksp_maxits);
  }

  size_t ls_its = 0;
  while (ls_its < _allowed_lambda_cuts)
  {
    _contact_lambda *= 0.5;
    /* update */
    ierr = VecWAXPY(W1, -_contact_lambda, Y, X);
    LIBMESH_CHKERR(ierr);

    ierr = SNESComputeFunction(snes, W1, G);
    LIBMESH_CHKERR(ierr);
    ierr = SNESGetFunctionDomainError(snes, &domainerror);
    LIBMESH_CHKERR(ierr);
    if (domainerror)
    {
      ierr = SNESLineSearchSetReason(line_search, SNES_LINESEARCH_FAILED_DOMAIN);
      LIBMESH_CHKERR(ierr);
    }
    ierr = VecNorm(G, NORM_2, &gnorm);
    LIBMESH_CHKERR(ierr);
    if (gnorm < fnorm)
    {
      VecCopy(G, F);
      LIBMESH_CHKERR(ierr);
      VecCopy(W1, W);
      LIBMESH_CHKERR(ierr);
      fnorm = gnorm;
      contact_state_stored.swap(_current_contact_state);
      _current_contact_state.clear();
      printContactInfo(contact_state_stored);
      ++ls_its;
    }
    else
      break;
  }

  ierr = VecScale(Y, _contact_lambda);
  LIBMESH_CHKERR(ierr);
  /* postcheck */
  ierr = SNESLineSearchPostCheck(line_search, X, Y, W, &changed_y, &changed_w);
  LIBMESH_CHKERR(ierr);

  if (changed_y)
  {
    ierr = VecWAXPY(W, -1., Y, X);
    LIBMESH_CHKERR(ierr);
  }

  if (changed_w || changed_y)
  {
    ierr = SNESComputeFunction(snes, W, F);
    LIBMESH_CHKERR(ierr);
    ierr = SNESGetFunctionDomainError(snes, &domainerror);
    LIBMESH_CHKERR(ierr);
    if (domainerror)
    {
      ierr = SNESLineSearchSetReason(line_search, SNES_LINESEARCH_FAILED_DOMAIN);
      LIBMESH_CHKERR(ierr);
    }
    contact_state_stored.swap(_current_contact_state);
    _current_contact_state.clear();
    printContactInfo(contact_state_stored);
  }

  /* copy the solution over */
  ierr = VecCopy(W, X);
  LIBMESH_CHKERR(ierr);

  ierr = SNESLineSearchComputeNorms(line_search);
  LIBMESH_CHKERR(ierr);

  ierr = VecDestroy(&W1);
  LIBMESH_CHKERR(ierr);

  _old_contact_state = std::move(contact_state_stored);
}
