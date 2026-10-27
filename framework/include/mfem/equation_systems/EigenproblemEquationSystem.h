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

#include <variant>

namespace Moose::MFEM
{
class EigensolverBase;

/// Scalar or matrix coefficient scaling the eigenproblem right-hand side. mfem::Coefficient and
/// mfem::MatrixCoefficient are unrelated types, so no single reference can express both.
using EigenRHSCoefficient = std::variant<mfem::Coefficient *, mfem::MatrixCoefficient *>;

/// Equation system specialization for eigenproblems.
class EigenproblemEquationSystem : public EquationSystem
{
public:
  EigenproblemEquationSystem() = default;
  ~EigenproblemEquationSystem() override = default;

  /// Build eigenproblem system, with essential boundary conditions accounted for
  void BuildEigenproblemJacobian(mfem::BlockVector & trueX, EigenRHSCoefficient rhs_coefficient);

  /// Prepare the provided eigensolver
  void PrepareEigensolver(EigensolverBase & solver);

  bool IsEigen() const override { return true; }

protected:
  /// Mark external boundaries as essential for eigenproblem BC elimination
  virtual void ApplyEssentialBCs() override;

  /// Verify that the problem is homogeneous (all Dirichlet BCs are zero)
  virtual void CheckProblemIsHomogeneous();

  /// Form HypreParMatrix matrix operator for the eigensolver with Dirichlet BC elimination.
  void FormEigenproblemMatrix();

  /// Form mass matrix for the eigensolver with Dirichlet BC elimination.
  void FormMassMatrix(EigenRHSCoefficient rhs_coefficient);

private:
  friend class EigenproblemESProblemOperator;

  /// The mass operator (e.g. the RHS operator for a generalized eigenproblem)
  mfem::OperatorHandle _mass_rhs;
};

} // namespace Moose::MFEM

#endif
