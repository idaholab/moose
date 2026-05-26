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
 * Base class for eigensolvers.
 */
class EigensolverBase : public LinearSolverBase
{
public:
  static InputParameters validParams();

  EigensolverBase(const InputParameters & parameters);

  /// Sets the operator for the eigensolver in derived classes
  virtual void setOperator(mfem::OperatorHandle & op) override = 0;

  /// Sets the mass matrix for the eigensolver in derived classes
  virtual void setMassMatrix(mfem::OperatorHandle & mass) = 0;

  /// Retrieves the computed eigenvalues
  virtual void getEigenvalues(mfem::Array<mfem::real_t> & eigenvalues) const = 0;

  /// Retrieves the computed eigenvector corresponding to the given index
  virtual const mfem::HypreParVector & getEigenvector(int index) const = 0;

  /// Updates the solver with the given bilinear form and essential dof list, in case an LOR or algebraic solver is needed.
  virtual void updateSolver(mfem::ParBilinearForm &, mfem::Array<int> &) override {}

protected:
  /// Number of eigenmodes to compute
  int _num_modes;
};
}

#endif
