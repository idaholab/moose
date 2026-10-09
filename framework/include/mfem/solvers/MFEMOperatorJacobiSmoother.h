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

#include "MFEMLORLinearSolverBase.h"

/**
 * Wrapper for mfem::OperatorJacobiSmoother solver.
 */
class MFEMOperatorJacobiSmoother
  : public Moose::MFEM::LORLinearSolverBase<mfem::OperatorJacobiSmoother>
{
public:
  static InputParameters validParams();

  MFEMOperatorJacobiSmoother(const InputParameters & parameters);

  void ConstructSolver() override;

protected:
  /// Update the wrapped MFEM solver parameters
  virtual void SetSolverParameters(mfem::OperatorJacobiSmoother & solver) override;
  /// Rebuild the smoother from the Schur complement diagonal when it is in use
  void SetOperatorImpl(mfem::Operator & op) override;

private:
  /// Whether to build the diagonal from an approximate Schur complement instead of the operator
  const bool _use_schur_complement;
  /// Essential true DoFs of schur_complement_variable. mfem::OperatorJacobiSmoother keeps a
  /// pointer to this array rather than a copy.
  mfem::Array<int> _ess_tdofs;
};

#endif
