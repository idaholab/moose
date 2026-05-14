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

#include "EquationSystem.h"

class MFEMEigensolverBase;

namespace Moose::MFEM
{

/// Equation system specialization for eigenproblems.
class EigenproblemEquationSystem : public EquationSystem
{
public:
  EigenproblemEquationSystem() = default;
  ~EigenproblemEquationSystem() override = default;

  /// Build eigenproblem system, with essential boundary conditions accounted for
  void BuildEigenproblemJacobian(mfem::BlockVector & trueX, mfem::OperatorHandle & massRHS);

protected:
  /// Mark external boundaries as essential for eigenproblem BC elimination
  virtual void ApplyEssentialBCs() override;

  /// Form HypreParMatrix matrix operator for the eigensolver with Dirichlet BC elimination.
  virtual void FormEigenproblemMatrix(mfem::OperatorHandle & op);

  /// Form mass matrix for the eigensolver with Dirichlet BC elimination.
  void FormMassMatrix(mfem::OperatorHandle & op);

private:
  friend class EigenproblemESProblemOperator;

  mfem::Array<int> _global_ess_markers;
};

} // namespace Moose::MFEM

#endif
