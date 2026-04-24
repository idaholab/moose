//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMLinearSolverBase.h"

namespace Moose::MFEM
{
/**
 * Wrapper for mfem::OperatorChebyshevSmoother.
 *
 * The smoother is symmetric positive definite by construction: MultTranspose == Mult and
 * the Chebyshev polynomial coefficients are chosen so that the preconditioned operator
 * remains positive on the estimated spectral interval. Unlike damped Jacobi, no manual
 * damping parameter is needed; the maximum eigenvalue is estimated automatically via a
 * power iteration on D^{-1}A at each updateSolver() call.
 *
 * Intended for use as a per-level smoother inside MFEMGeometricMultigridSolver.
 */
class OperatorChebyshevSmoother : public LinearSolverBase
{
public:
  static InputParameters validParams();

  OperatorChebyshevSmoother(const InputParameters & parameters);

  /// Builds (or rebuilds) the smoother from the operator and essential tdofs.
  /// Must be called before getSolver(). Called by GeometricMultigridSolver once per level per solve.
  void updateSolver(mfem::Operator & op, mfem::Array<int> & tdofs) override;

protected:
  void constructSolver() override;

private:
  const int _order;
  mfem::Vector _diag; ///< Assembled diagonal; kept as a member so it outlives the smoother.
};

} // namespace Moose::MFEM
#endif
