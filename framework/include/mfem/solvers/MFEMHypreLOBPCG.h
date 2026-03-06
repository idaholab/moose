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

#include "MFEMEigensolverBase.h"

/**
 * Class for the Hypre LOBPCG eigensolver
 */
class MFEMHypreLOBPCG : public MFEMEigensolverBase
{
public:
  static InputParameters validParams();

  MFEMHypreLOBPCG(const InputParameters & parameters);

  /// Updates the solver with the given bilinear form and essential dof list.
  virtual void updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs) override;

  /// Sets the operator for the eigensolver
  virtual void setOperator(mfem::Operator & op) override { _eigensolver->SetOperator(op); }

  /// Sets the mass matrix for the eigensolver
  virtual void setMassMatrix(mfem::Operator & mass) override { _eigensolver->SetMassMatrix(mass); }

  /// Solves the eigenvalue problem
  virtual void Solve() override { _eigensolver->Solve(); }

  /// Retrieves the computed eigenvalues
  virtual void getEigenvalues(mfem::Array<mfem::real_t> & eigenvalues) const override
  {
    _eigensolver->GetEigenvalues(eigenvalues);
  }

  /// Retrieves the computed eigenvector corresponding to the given index
  virtual const mfem::HypreParVector & getEigenvector(int index) const override
  {
    return _eigensolver->GetEigenvector(index);
  }

protected:
  /// Override in derived classes to construct and set the solver options.
  virtual void constructSolver(const InputParameters &) override;

  /// Eigensolver to be used for the problem
  std::unique_ptr<mfem::HypreLOBPCG> _eigensolver;

  /// Mass matrix for eigensolver
  std::unique_ptr<mfem::HypreParMatrix> _M;

  /// Mass matrix coefficient
  mfem::Coefficient & _coef;
};

#endif
