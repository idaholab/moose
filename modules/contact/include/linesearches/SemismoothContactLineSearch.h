//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LineSearch.h"

#include "libmesh/petsc_nonlinear_solver.h"

#include <deque>

/**
 * Backtracking line search for semismooth Newton on the Fischer-Burmeister
 * reformulation used by PETSc `SNESVINEWTONSSLS`.
 *
 * When invoked from inside an SSLS solve, `SNESComputeFunction(snes, W, F)`
 * returns the FB-reformulated function \f$ \Phi(W) \f$ rather than the raw
 * physics residual (SSLS swaps the SNES function op during its solve). This
 * class exploits that: the merit driven down by the backtracking loop is
 * \f$ \psi(x) = \tfrac{1}{2}\|\Phi(x)\|^2 \f$.
 *
 * Acceptance uses a *nonmonotone Armijo* rule (Grippo, Lampariello, Lucidi
 * 1986): a trial step is accepted if
 *
 *   \psi(x + \lambda d) \le \psi_{\text{ref}} \cdot (1 - c\,\lambda)
 *
 * where \f$ \psi_{\text{ref}} = \max\{\psi_{k-j} : 0 \le j < M\} \f$ over a
 * rolling window of the last M accepted merits. This lets a Newton step whose
 * merit temporarily rises (as multiple LM DoFs cross the FB kink at
 * \f$(\lambda_i = 0, g_i = 0)\f$ together in an active-set flip) still pass
 * the line-search test, while long-run monotone decrease is still enforced.
 *
 * Meant to be paired with `-snes_type vinewtonssls` and `ConstantBounds` on
 * the contact LM variable. Under `newtonls`, this behaves as a plain
 * nonmonotone Armijo line search on the physics residual (still useful, but
 * loses the FB rationale).
 */
class SemismoothContactLineSearch : public LineSearch
{
public:
  static InputParameters validParams();

  SemismoothContactLineSearch(const InputParameters & parameters);

  virtual void lineSearch() override;

  virtual void timestepSetup() override;

protected:
  /// Nonlinear solver -- cached at construction to reach the SNES.
  libMesh::PetscNonlinearSolver<Real> * _solver;

  /// Armijo constant c in the sufficient-decrease test.
  const Real _armijo_c;

  /// Backtracking factor rho (0 < rho < 1); lambda <- rho * lambda on each cut.
  const Real _backtrack_rho;

  /// Maximum number of backtracks (lambda_min = rho^max_cuts).
  const unsigned int _max_cuts;

  /// Nonmonotone window size M (set M = 1 for classical monotone Armijo).
  const unsigned int _window;

  /// Rolling window of the M most recently accepted merit values (1/2||Phi||^2).
  std::deque<Real> _recent_meritsq;
};
