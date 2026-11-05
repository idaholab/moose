//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SemismoothContactLineSearch.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"
#include "libmesh/petsc_solver_exception.h"

#include <petscsnes.h>
#include <petscversion.h>

#include <algorithm>

registerMooseObject("ContactApp", SemismoothContactLineSearch);

InputParameters
SemismoothContactLineSearch::validParams()
{
  InputParameters params = LineSearch::validParams();
  params.addClassDescription(
      "Backtracking line search on the semismooth (Fischer-Burmeister) merit "
      "used by PETSc SNESVINEWTONSSLS. Nonmonotone Armijo acceptance permits "
      "temporary merit increases during active-set flips.");
  params.addRangeCheckedParam<Real>("armijo_constant",
                                    1.0e-4,
                                    "armijo_constant > 0 & armijo_constant < 1",
                                    "Armijo sufficient-decrease constant c in psi(x + lambda*d) <= "
                                    "psi_ref * (1 - c*lambda).");
  params.addRangeCheckedParam<Real>(
      "backtrack_factor",
      0.5,
      "backtrack_factor > 0 & backtrack_factor < 1",
      "Backtracking factor rho; each rejected step scales lambda by rho.");
  params.addRangeCheckedParam<unsigned int>(
      "max_cuts", 10, "max_cuts > 0", "Maximum number of backtracks.");
  params.addRangeCheckedParam<unsigned int>(
      "nonmonotone_window",
      5,
      "nonmonotone_window > 0",
      "Rolling-window size M for the nonmonotone reference merit. Set to 1 "
      "for classical monotone Armijo.");
  return params;
}

SemismoothContactLineSearch::SemismoothContactLineSearch(const InputParameters & parameters)
  : LineSearch(parameters),
    _armijo_c(getParam<Real>("armijo_constant")),
    _backtrack_rho(getParam<Real>("backtrack_factor")),
    _max_cuts(getParam<unsigned int>("max_cuts")),
    _window(getParam<unsigned int>("nonmonotone_window"))
{
  _solver = dynamic_cast<libMesh::PetscNonlinearSolver<Real> *>(
      _fe_problem.getNonlinearSystem(/*nl_sys_num=*/0).nonlinearSolver());
  if (!_solver)
    mooseError("SemismoothContactLineSearch requires the PETSc nonlinear solver.");
}

void
SemismoothContactLineSearch::timestepSetup()
{
  // Reset the nonmonotone window between load steps so an old, stale merit
  // does not distort acceptance decisions for the new step.
  _recent_meritsq.clear();
}

void
SemismoothContactLineSearch::lineSearch()
{
  PetscBool changed_y = PETSC_FALSE, changed_w = PETSC_FALSE;
  Vec X, F, Y, W, G, W_try;
  SNESLineSearch line_search;
  PetscReal fnorm = 0.0, xnorm = 0.0, ynorm = 0.0;
  SNES snes = _solver->snes();

  LibmeshPetscCall(SNESGetLineSearch(snes, &line_search));
  LibmeshPetscCall(SNESLineSearchGetVecs(line_search, &X, &F, &Y, &W, &G));
  LibmeshPetscCall(SNESLineSearchGetNorms(line_search, &xnorm, &fnorm, &ynorm));
  LibmeshPetscCall(SNESLineSearchGetSNES(line_search, &snes));
  LibmeshPetscCall(SNESLineSearchSetReason(line_search, SNES_LINESEARCH_SUCCEEDED));
  LibmeshPetscCall(VecDuplicate(W, &W_try));

  ++_nl_its;

  // Precheck (allows pre/post-check callbacks to modify Y).
  LibmeshPetscCall(SNESLineSearchPreCheck(line_search, X, Y, &changed_y));

  // ------------------------------------------------------------------
  // Reference merit for nonmonotone acceptance.  fnorm on entry is
  // ||F(X)|| = ||Phi(X)|| under SSLS; use its square/2 for psi_ref.
  // ------------------------------------------------------------------
  const Real psi_curr = 0.5 * fnorm * fnorm;
  Real psi_ref = psi_curr;
  for (const auto psi_past : _recent_meritsq)
    psi_ref = std::max(psi_ref, psi_past);

  auto try_step = [&](Real lambda, Vec Wout, PetscReal & merit_norm) -> bool
  {
    LibmeshPetscCall(VecWAXPY(Wout, -lambda, Y, X));
    LibmeshPetscCall(SNESComputeFunction(snes, Wout, F));

    // Domain-error propagation (matches PetscContactLineSearch).
#if PETSC_VERSION_LESS_THAN(3, 25, 0)
    PetscBool domainerror;
    LibmeshPetscCall(SNESGetFunctionDomainError(snes, &domainerror));
    if (domainerror)
    {
      LibmeshPetscCall(SNESLineSearchSetReason(line_search, SNES_LINESEARCH_FAILED_DOMAIN));
      return false;
    }
#else
    SNESConvergedReason reason;
    LibmeshPetscCall(SNESGetConvergedReason(snes, &reason));
    if (reason == SNES_DIVERGED_FUNCTION_DOMAIN)
    {
      LibmeshPetscCall(
          SNESLineSearchSetReason(line_search, SNES_LINESEARCH_FAILED_FUNCTION_DOMAIN));
      return false;
    }
#endif

    LibmeshPetscCall(VecNorm(F, NORM_2, &merit_norm));
    return true;
  };

  // ------------------------------------------------------------------
  // Backtracking loop with nonmonotone Armijo test on psi = 1/2 ||Phi||^2.
  // ------------------------------------------------------------------
  Real lambda = 1.0;
  PetscReal try_norm = 0.0;
  bool step_ok = try_step(lambda, W_try, try_norm);
  bool accepted = false;
  unsigned int cuts = 0;

  if (step_ok)
  {
    const Real psi_try = 0.5 * try_norm * try_norm;
    accepted = psi_try <= psi_ref * (1.0 - _armijo_c * lambda);
  }

  while (!accepted && cuts < _max_cuts)
  {
    lambda *= _backtrack_rho;
    step_ok = try_step(lambda, W_try, try_norm);
    if (!step_ok)
      break;

    const Real psi_try = 0.5 * try_norm * try_norm;
    accepted = psi_try <= psi_ref * (1.0 - _armijo_c * lambda);
    ++cuts;
  }

  // If we never accepted, take the smallest-lambda trial as a best-effort
  // step: semismooth Newton on FB can tolerate an occasional non-decrease,
  // and dt cutbacks / SNES retries provide a safety net at the outer loop.
  if (!accepted)
  {
    _console << "SemismoothContactLineSearch: sufficient-decrease test failed after " << cuts
             << " backtracks; accepting lambda = " << lambda << " (merit "
             << 0.5 * try_norm * try_norm << " vs ref " << psi_ref << ")." << std::endl;
  }

  // ------------------------------------------------------------------
  // Commit the accepted step.  Copy the trial state (W_try, F) back into
  // the SNES-owned (W, F) and update Y so SNES sees the effective step.
  // ------------------------------------------------------------------
  LibmeshPetscCall(VecCopy(W_try, W));
  fnorm = try_norm;

  LibmeshPetscCall(VecScale(Y, lambda));

  // Postcheck (allows callers to further modify Y or W).
  LibmeshPetscCall(SNESLineSearchPostCheck(line_search, X, Y, W, &changed_y, &changed_w));
  if (changed_y)
    LibmeshPetscCall(VecWAXPY(W, -1.0, Y, X));
  if (changed_w || changed_y)
  {
    LibmeshPetscCall(SNESComputeFunction(snes, W, F));
    LibmeshPetscCall(VecNorm(F, NORM_2, &fnorm));
  }

  // Commit to X, refresh SNES-side norms.
  LibmeshPetscCall(VecCopy(W, X));
  LibmeshPetscCall(SNESLineSearchComputeNorms(line_search));

  // Rolling window bookkeeping.
  _recent_meritsq.push_back(0.5 * fnorm * fnorm);
  while (_recent_meritsq.size() > _window)
    _recent_meritsq.pop_front();

  LibmeshPetscCall(VecDestroy(&W_try));
}
