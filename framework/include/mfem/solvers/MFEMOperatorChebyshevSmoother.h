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

/**
 * Wrapper for mfem::OperatorChebyshevSmoother.
 *
 * The smoother is symmetric positive definite by construction: MultTranspose == Mult and
 * the Chebyshev polynomial coefficients are chosen so that the preconditioned operator
 * remains positive on the estimated spectral interval. Unlike damped Jacobi, no manual
 * damping parameter is needed; the maximum eigenvalue is estimated automatically via a
 * power iteration on D^{-1}A at each SetOperator() call.
 *
 * An example use of this class is as a smoother inside MFEMGeometricMultigridSolver.
 */
class MFEMOperatorChebyshevSmoother : public Moose::MFEM::LinearSolverBase
{
public:
  static InputParameters validParams();

  MFEMOperatorChebyshevSmoother(const InputParameters & parameters);

protected:
  void ConstructSolver() override;

  /// Rebuilds the multigrid hierarchy for the supplied finest-level operator.
  virtual void SetOperatorImpl(mfem::Operator & op) override;

private:
  /// Degree of the Chebyshev polynomial used by the MFEM smoother.
  const int _order;

  /// Assembled diagonal
  mfem::Vector _diag;

  /// The operator supplied by MOOSE is already constrained.
  mfem::Array<int> _empty_tdofs;
};
#endif
