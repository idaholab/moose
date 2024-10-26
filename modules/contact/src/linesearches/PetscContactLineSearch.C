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
      _fe_problem.getNonlinearSystem(/*nl_sys_num=*/0).nonlinearSolver());
  if (!_solver)
    mooseError(
        "This line search operates only with Petsc, so Petsc must be your nonlinear solver.");
}

void
PetscContactLineSearch::lineSearch()
{
  PetscBool changed_y = PETSC_FALSE, changed_w = PETSC_FALSE;
  Vec X, F, Y, W, G, W1;
  SNESLineSearch line_search;
  PetscReal fnorm, xnorm, ynorm, gnorm;
  PetscBool domainerror;
  PetscReal ksp_rtol, ksp_abstol, ksp_dtol;
  PetscInt ksp_maxits;
  KSP ksp;
  SNES snes = _solver->snes();

  LibmeshPetscCall(SNESGetLineSearch(snes, &line_search));
  LibmeshPetscCall(SNESLineSearchGetVecs(line_search, &X, &F, &Y, &W, &G));
  LibmeshPetscCall(SNESLineSearchGetNorms(line_search, &xnorm, &fnorm, &ynorm));
  LibmeshPetscCall(SNESLineSearchGetSNES(line_search, &snes));
  LibmeshPetscCall(SNESLineSearchSetReason(line_search, SNES_LINESEARCH_SUCCEEDED));
  LibmeshPetscCall(SNESGetKSP(snes, &ksp));
  LibmeshPetscCall(KSPGetTolerances(ksp, &ksp_rtol, &ksp_abstol, &ksp_dtol, &ksp_maxits));
  LibmeshPetscCall(VecDuplicate(W, &W1));

  if (!_user_ksp_rtol_set)
  {
    _user_ksp_rtol = ksp_rtol;
    _user_ksp_rtol_set = true;
  }

  ++_nl_its;

  /* precheck */
  LibmeshPetscCall(SNESLineSearchPreCheck(line_search, X, Y, &changed_y));

  /* temporary update */
  _contact_lambda = 1.;
  LibmeshPetscCall(VecWAXPY(W, -_contact_lambda, Y, X));

  /* compute residual to determine whether contact state has changed since the last non-linear
   * residual evaluation */
  _current_contact_state.clear();
  LibmeshPetscCall(SNESComputeFunction(snes, W, F));
  LibmeshPetscCall(SNESGetFunctionDomainError(snes, &domainerror));
  if (domainerror)
    LibmeshPetscCall(SNESLineSearchSetReason(line_search, SNES_LINESEARCH_FAILED_DOMAIN));

  LibmeshPetscCall(VecNorm(F, NORM_2, &fnorm));
  std::set<dof_id_type> contact_state_stored = _current_contact_state;
  _current_contact_state.clear();
  printContactInfo(contact_state_stored);

  if (_affect_ltol)
  {
    if (contact_state_stored != _old_contact_state)
    {
      LibmeshPetscCall(KSPSetTolerances(ksp, _contact_ltol, ksp_abstol, ksp_dtol, ksp_maxits));
      _console << "Contact set changed since previous non-linear iteration!" << std::endl;
    }
    else
      LibmeshPetscCall(KSPSetTolerances(ksp, _user_ksp_rtol, ksp_abstol, ksp_dtol, ksp_maxits));
  }

  size_t ls_its = 0;
  while (ls_its < _allowed_lambda_cuts)
  {
    _contact_lambda *= 0.5;
    /* update */
    LibmeshPetscCall(VecWAXPY(W1, -_contact_lambda, Y, X));

    LibmeshPetscCall(SNESComputeFunction(snes, W1, G));
    LibmeshPetscCall(SNESGetFunctionDomainError(snes, &domainerror));
    if (domainerror)
      LibmeshPetscCall(SNESLineSearchSetReason(line_search, SNES_LINESEARCH_FAILED_DOMAIN));

    LibmeshPetscCall(VecNorm(G, NORM_2, &gnorm));
    if (gnorm < fnorm)
    {
      LibmeshPetscCall(VecCopy(G, F));
      LibmeshPetscCall(VecCopy(W1, W));
      fnorm = gnorm;
      contact_state_stored.swap(_current_contact_state);
      _current_contact_state.clear();
      printContactInfo(contact_state_stored);
      ++ls_its;
    }
    else
      break;
  }

  LibmeshPetscCall(VecScale(Y, _contact_lambda));
  /* postcheck */
  LibmeshPetscCall(SNESLineSearchPostCheck(line_search, X, Y, W, &changed_y, &changed_w));

  if (changed_y)
    LibmeshPetscCall(VecWAXPY(W, -1., Y, X));

  if (changed_w || changed_y)
  {
    LibmeshPetscCall(SNESComputeFunction(snes, W, F));
    LibmeshPetscCall(SNESGetFunctionDomainError(snes, &domainerror));
    if (domainerror)
      LibmeshPetscCall(SNESLineSearchSetReason(line_search, SNES_LINESEARCH_FAILED_DOMAIN));

    contact_state_stored.swap(_current_contact_state);
    _current_contact_state.clear();
    printContactInfo(contact_state_stored);
  }

  /* copy the solution over */
  LibmeshPetscCall(VecCopy(W, X));

  LibmeshPetscCall(SNESLineSearchComputeNorms(line_search));

  LibmeshPetscCall(VecDestroy(&W1));

  _old_contact_state = std::move(contact_state_stored);
}
