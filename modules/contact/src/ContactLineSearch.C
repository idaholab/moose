//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ContactLineSearch.h"

#ifdef LIBMESH_HAVE_PETSC

#include "FEProblemBase.h"
#include "libmesh/petsc_solver_exception.h"
#include "petsc/private/linesearchimpl.h"

ContactLineSearch::ContactLineSearch(FEProblemBase & fe_problem,
                                     MooseApp & app,
                                     size_t allowed_lambda_cuts)
  : ConsoleStreamInterface(app),
    ParallelObject(app),
    _fe_problem(fe_problem),
    _nl_its(0),
    _user_ksp_rtol_set(false),
    _allowed_lambda_cuts(allowed_lambda_cuts)
{
}

void
ContactLineSearch::linesearch(SNESLineSearch linesearch)
{
  PetscBool changed_y = PETSC_FALSE, changed_w = PETSC_FALSE;
  PetscErrorCode ierr;
  Vec X, F, Y, W, G, W1;
  SNES snes;
  PetscReal fnorm, xnorm, ynorm, gnorm;
  PetscBool domainerror;
  PetscReal ksp_rtol, ksp_abstol, ksp_dtol;
  PetscInt ksp_maxits;
  KSP ksp;

  ierr = SNESLineSearchGetVecs(linesearch, &X, &F, &Y, &W, &G);
  LIBMESH_CHKERR(ierr);
  ierr = SNESLineSearchGetNorms(linesearch, &xnorm, &fnorm, &ynorm);
  LIBMESH_CHKERR(ierr);
  ierr = SNESLineSearchGetSNES(linesearch, &snes);
  LIBMESH_CHKERR(ierr);
  ierr = SNESLineSearchSetReason(linesearch, SNES_LINESEARCH_SUCCEEDED);
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
  ierr = SNESLineSearchPreCheck(linesearch, X, Y, &changed_y);
  LIBMESH_CHKERR(ierr);

  /* temporary update */
  _contact_lambda = 1.;
  ierr = VecWAXPY(W, -_contact_lambda, Y, X);
  LIBMESH_CHKERR(ierr);

  /* compute residual to determine whether contact state has changed since the last non-linear
   * residual evaluation */
  _current_contact_state.clear();
  ierr = (*linesearch->ops->snesfunc)(snes, W, F);
  LIBMESH_CHKERR(ierr);
  ierr = SNESGetFunctionDomainError(snes, &domainerror);
  LIBMESH_CHKERR(ierr);
  if (domainerror)
  {
    ierr = SNESLineSearchSetReason(linesearch, SNES_LINESEARCH_FAILED_DOMAIN);
    LIBMESH_CHKERR(ierr);
  }
  ierr = VecNorm(F, NORM_2, &fnorm);
  LIBMESH_CHKERR(ierr);
  _communicator.set_union(_current_contact_state);
  std::set<dof_id_type> contact_state_stored = _current_contact_state;
  printContactInfo();

  if (_current_contact_state != _old_contact_state)
  {
    KSPSetTolerances(ksp, .5, ksp_abstol, ksp_dtol, ksp_maxits);
    _console << "Contact set changed since previous non-linear iteration!\n";
  }
  else
    KSPSetTolerances(ksp, _user_ksp_rtol, ksp_abstol, ksp_dtol, ksp_maxits);

  size_t ls_its = 0;
  while (ls_its < _allowed_lambda_cuts)
  {
    _contact_lambda *= 0.5;
    /* update */
    ierr = VecWAXPY(W1, -_contact_lambda, Y, X);
    LIBMESH_CHKERR(ierr);

    _current_contact_state.clear();
    ierr = (*linesearch->ops->snesfunc)(snes, W1, G);
    LIBMESH_CHKERR(ierr);
    ierr = SNESGetFunctionDomainError(snes, &domainerror);
    LIBMESH_CHKERR(ierr);
    if (domainerror)
    {
      ierr = SNESLineSearchSetReason(linesearch, SNES_LINESEARCH_FAILED_DOMAIN);
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
      _communicator.set_union(_current_contact_state);
      contact_state_stored = _current_contact_state;
      printContactInfo();
      ++ls_its;
    }
    else
      break;
  }

  ierr = VecScale(Y, _contact_lambda);
  LIBMESH_CHKERR(ierr);
  /* postcheck */
  ierr = SNESLineSearchPostCheck(linesearch, X, Y, W, &changed_y, &changed_w);
  LIBMESH_CHKERR(ierr);

  if (changed_y)
  {
    ierr = VecWAXPY(W, -1., Y, X);
    LIBMESH_CHKERR(ierr);
  }

  if (changed_w || changed_y)
  {
    _current_contact_state.clear();
    ierr = (*linesearch->ops->snesfunc)(snes, W, F);
    LIBMESH_CHKERR(ierr);
    ierr = SNESGetFunctionDomainError(snes, &domainerror);
    LIBMESH_CHKERR(ierr);
    if (domainerror)
    {
      ierr = SNESLineSearchSetReason(linesearch, SNES_LINESEARCH_FAILED_DOMAIN);
      LIBMESH_CHKERR(ierr);
    }
    _communicator.set_union(_current_contact_state);
    contact_state_stored = _current_contact_state;
    printContactInfo();
  }

  ierr = VecNorm(Y, NORM_2, &linesearch->ynorm);
  LIBMESH_CHKERR(ierr);
  ierr = VecNorm(W, NORM_2, &linesearch->xnorm);
  LIBMESH_CHKERR(ierr);
  ierr = VecNorm(F, NORM_2, &linesearch->fnorm);
  LIBMESH_CHKERR(ierr);

  /* copy the solution over */
  ierr = VecCopy(W, X);
  LIBMESH_CHKERR(ierr);

  ierr = VecDestroy(&W1);
  LIBMESH_CHKERR(ierr);

  _old_contact_state = contact_state_stored;
}

void
ContactLineSearch::printContactInfo()
{
  if (!_current_contact_state.empty())
  {
    // _console << "Node ids in contact: ";
    // for (auto & node_id : _current_contact_state)
    //   _console << node_id << " ";
    // _console << "\n";
    _console << _current_contact_state.size() << " nodes in contact\n";
  }
  else
    _console << "No nodes in contact\n";
}

#endif
