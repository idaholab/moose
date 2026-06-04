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
class MFEMEigenproblem;

namespace Moose::MFEM
{
class EigensolverBase;

/// Equation system specialization for eigenproblems.
class EigenproblemEquationSystem : public EquationSystem
{
public:
  /// Construct with the eigenproblem owning the right-hand-side coefficient. The coefficient is
  /// resolved at assembly time, since materials are not yet available when the equation
  /// system is created.
  EigenproblemEquationSystem(MFEMEigenproblem & eigen_problem) : _eigen_problem(eigen_problem) {}
  ~EigenproblemEquationSystem() override = default;

  /// Build eigenproblem system, with essential boundary conditions accounted for
  void BuildEigenproblemJacobian(mfem::BlockVector & trueX);

  /// Prepare the provided eigensolver
  void PrepareEigensolver(EigensolverBase & solver);

protected:
  /// Mark external boundaries as essential for eigenproblem BC elimination
  virtual void ApplyEssentialBCs() override;

  /// Form HypreParMatrix matrix operator for the eigensolver with Dirichlet BC elimination.
  void FormEigenproblemMatrix();

  /// Form mass matrix for the eigensolver with Dirichlet BC elimination.
  void FormMassMatrix();

private:
  friend class EigenproblemESProblemOperator;

  mfem::Array<int> _global_ess_markers;
  /// The mass operator (e.g. the RHS operator for a generalized eigenproblem)
  mfem::OperatorHandle _mass_rhs;
  mfem::Coefficient & _rhs_coef;
  /// Eigenproblem owning the right-hand-side coefficient, queried at assembly time.
  MFEMEigenproblem & _eigen_problem;
};

} // namespace Moose::MFEM

#endif
