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

#include "ComplexEquationSystem.h"

class MFEMEigensolverBase;
class MFEMEigenproblem;

namespace Moose::MFEM
{

/// Equation system specialization for complex eigenproblems.
class ComplexEigenproblemEquationSystem : public ComplexEquationSystem
{
public:
  /// Construct with the eigenproblem owning the right-hand-side coefficient. The coefficient is
  /// resolved at assembly time, since materials are not yet available when the equation
  /// system is created.
  ComplexEigenproblemEquationSystem(MFEMEigenproblem & eigen_problem)
    : _eigen_problem(eigen_problem)
  {
  }
  ~ComplexEigenproblemEquationSystem() override = default;

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
  friend class ComplexEigenproblemESProblemOperator;

  mfem::Array<int> _global_ess_markers;
  /// Eigenproblem owning the right-hand-side coefficient, queried at assembly time.
  MFEMEigenproblem & _eigen_problem;
};

} // namespace Moose::MFEM

#endif
