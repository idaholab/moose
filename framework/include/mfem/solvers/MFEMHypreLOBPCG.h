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
class MFEMHypreLOBPCG : public Moose::MFEM::EigensolverBase
{
public:
  static InputParameters validParams();

  MFEMHypreLOBPCG(const InputParameters & parameters);

  /// Sets the operator for the eigensolver and propagates it to the preconditioner.
  virtual void SetOperator(mfem::Operator & op) override
  {
    if (_preconditioner)
      _preconditioner->SetOperator(op);
    _eigensolver->SetOperator(op);
  }

  /// Sets the mass matrix for the eigensolver
  virtual void SetMassMatrix(mfem::Operator & mass) override { _eigensolver->SetMassMatrix(mass); }

  /// Solves the eigenvalue problem
  virtual void Solve() override { _eigensolver->Solve(); }

  /// Retrieves the computed eigenvalues
  virtual void GetEigenvalues(mfem::Array<mfem::real_t> & eigenvalues) const override
  {
    mfem::Array<mfem::real_t> computed;
    _eigensolver->GetEigenvalues(computed);
    eigenvalues.SetSize(_num_modes);
    for (int i = 0; i < _num_modes; ++i)
      eigenvalues[i] = computed[i * _mode_stride];
  }

  /// Retrieves the computed eigenvector corresponding to the given index
  virtual const mfem::HypreParVector & GetEigenvector(int index) const override
  {
    return _eigensolver->GetEigenvector(index * _mode_stride);
  }

protected:
  /// Override in derived classes to construct and set the solver options.
  virtual void ConstructSolver() override;

  /// Eigensolver to be used for the problem
  std::unique_ptr<mfem::HypreLOBPCG> _eigensolver;
};

#endif
