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
 * Base class for eigensolvers.
 */
class MFEMEigensolverBase : public MFEMSolverBase
{
public:
  static InputParameters validParams();

  MFEMEigensolverBase(const InputParameters & parameters);

  /// Sets the operator for the eigensolver in derived classes
  virtual void setOperator(mfem::Operator & op) = 0;

  /// Returns whether or not this solver is an eigensolver
  virtual bool isEigensolver() const override { return true; }

  /// Retrieves the computed eigenvalues
  virtual void GetEigenvalues(mfem::Array<mfem::real_t> & eigenvalues) const = 0;

protected:
  /// Mass matrix for eigensolver
  std::unique_ptr<mfem::HypreParMatrix> _M;
};

#endif
