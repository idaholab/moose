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

#include "MFEMSolverBase.h"

/**
 * Base class for eigensolvers. It needs to be templated due to the Hypre eigensolvers not deriving from mfem::HypreSolver.
 */
template <typename T>
class MFEMEigensolverBase : public MFEMSolverBase
{
public:
  static InputParameters validParams();

  MFEMEigensolverBase(const InputParameters & parameters);

  /// Updates the solver with the given bilinear form and essential dof list.
  virtual void updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs) override;

  /// Returns whether or not this solver is an eigensolver
  virtual bool isEigensolver() const override { return true; }

  /// Solves the eigenvalue problem
  virtual void Solve() override { _eigensolver->Solve(); }

protected:

  /// Override in derived classes to construct and set the solver options.
  virtual void constructSolver(const InputParameters &) override;

  /// Eigensolver to be used for the problem
  std::unique_ptr<T> _eigensolver;

  /// Mass matrix for eigensolver
  std::unique_ptr<mfem::HypreParMatrix> _M;

};

#endif
