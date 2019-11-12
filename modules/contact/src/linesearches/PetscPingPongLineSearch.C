//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PetscPingPongLineSearch.h"

#ifdef LIBMESH_HAVE_PETSC
#if !PETSC_VERSION_LESS_THAN(3, 6, 0)
#include "FEProblem.h"
#include "NonlinearSystem.h"
#include "libmesh/petsc_nonlinear_solver.h"
#include "libmesh/petsc_solver_exception.h"
#include <petscdm.h>

registerMooseObject("ContactApp", PetscPingPongLineSearch);

template <>
InputParameters
validParams<PetscPingPongLineSearch>()
{
  auto params = validParams<LineSearch>();
  params.addParam<Real>(
      "ping_pong_tol", 1e-4, "A relative tolerance for determining whether we are ping-ponging");
  return params;
}

PetscPingPongLineSearch::PetscPingPongLineSearch(const InputParameters & parameters)
  : LineSearch(parameters),
    _fnorm_older(0),
    _fnorm_old(0),
    _ping_pong_tol(getParam<Real>("ping_pong_tol"))
{
  _solver = dynamic_cast<PetscNonlinearSolver<Real> *>(
      _fe_problem.getNonlinearSystem().nonlinearSolver());
  if (!_solver)
    mooseError(
        "This line search operates only with Petsc, so Petsc must be your nonlinear solver.");
}

void
PetscPingPongLineSearch::timestepSetup()
{
  _nl_its = 0;
}

void
PetscPingPongLineSearch::lineSearch()
{
  PetscBool changed_y = PETSC_FALSE, changed_w = PETSC_FALSE;
  PetscErrorCode ierr;
  Vec X, F, Y, W, G;
  SNESLineSearch line_search;
  PetscReal fnorm, xnorm, ynorm;
  PetscBool domainerror;
  SNES snes = _solver->snes();

  ierr = SNESGetLineSearch(snes, &line_search);
  LIBMESH_CHKERR(ierr);
  ierr = SNESLineSearchGetVecs(line_search, &X, &F, &Y, &W, &G);
  LIBMESH_CHKERR(ierr);
  ierr = SNESLineSearchGetNorms(line_search, &xnorm, &fnorm, &ynorm);
  LIBMESH_CHKERR(ierr);
  ierr = SNESLineSearchSetReason(line_search, SNES_LINESEARCH_SUCCEEDED);
  LIBMESH_CHKERR(ierr);

  ++_nl_its;

  ierr = SNESLineSearchPreCheck(line_search, X, Y, &changed_y);
  LIBMESH_CHKERR(ierr);

  // basic line search
  _lambda = 1.;
  ierr = VecWAXPY(W, -_lambda, Y, X);
  LIBMESH_CHKERR(ierr);

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

  // Check for ping pong. If we're ping-ponging then we simply take half the step
  if (_nl_its >= 2 && std::abs(fnorm - _fnorm_older) / _fnorm_older < _ping_pong_tol)
  {
    // basic line search
    _lambda = 0.5;
    ierr = VecWAXPY(W, -_lambda, Y, X);
    LIBMESH_CHKERR(ierr);

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

    _console << "Cutting lambda\n";
  }

  ierr = VecScale(Y, _lambda);
  LIBMESH_CHKERR(ierr);
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
    ierr = VecNorm(F, NORM_2, &fnorm);
    LIBMESH_CHKERR(ierr);
  }

  ierr = VecCopy(W, X);
  LIBMESH_CHKERR(ierr);

  ierr = SNESLineSearchComputeNorms(line_search);
  LIBMESH_CHKERR(ierr);

  _fnorm_older = _fnorm_old;
  _fnorm_old = fnorm;
}

#endif // !PETSC_VERSION_LESS_THAN(3, 3, 0)
#endif // LIBMESH_HAVE_PETSC
