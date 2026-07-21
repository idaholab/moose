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
 * Class for the HypreAME eigensolver
 */
class MFEMHypreAME : public Moose::MFEM::EigensolverBase
{
public:
  static InputParameters validParams();

  MFEMHypreAME(const InputParameters & parameters);

  /// Sets the operator for the eigensolver and propagates it to the preconditioner.
  virtual void SetOperator(mfem::Operator & op) override
  {
    if (_preconditioner)
      _preconditioner->SetOperator(op);
    _eigensolver->SetOperator(libMesh::cast_ref<mfem::HypreParMatrix &>(op));
  }

  /// Sets the mass matrix for the eigensolver
  virtual void SetMassMatrix(mfem::Operator & mass) override
  {
    _eigensolver->SetMassMatrix(libMesh::cast_ref<mfem::HypreParMatrix &>(mass));
  }

  /// Solves the eigenvalue problem
  virtual void Solve() override { _eigensolver->Solve(); }

  /// Retrieves the computed eigenvalues
  virtual void GetEigenvalues(mfem::Array<mfem::real_t> & eigenvalues) const override
  {
    _eigensolver->GetEigenvalues(eigenvalues);
  }

  /// Retrieves the computed eigenvector corresponding to the given index
  virtual const mfem::HypreParVector & GetEigenvector(int index) const override
  {
    return _eigensolver->GetEigenvector(index);
  }

protected:
  /// Override in derived classes to construct and set the solver options.
  virtual void ConstructSolver() override;

  /// Eigensolver to be used for the problem
  std::unique_ptr<mfem::HypreAME> _eigensolver;
};

#endif
