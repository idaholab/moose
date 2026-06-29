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

#include "ComplexEquationSystemProblemOperator.h"
#include "ComplexEigenproblemEquationSystem.h"

namespace Moose::MFEM
{

class EigensolverBase;

/// Steady-state eigenproblem operator for complex (Hermitian) equation systems.
class ComplexEigenproblemESProblemOperator : public ComplexEquationSystemProblemOperator
{
public:
  ComplexEigenproblemESProblemOperator(MFEMProblem & problem)
    : ComplexEquationSystemProblemOperator(problem)
  {
    mooseAssert(
        std::dynamic_pointer_cast<ComplexEigenproblemEquationSystem>(_problem_data.eqn_system),
        "MFEMProblemData::eqn_system is not a ComplexEigenproblemEquationSystem.");
  }

  virtual void Solve() override;

  mfem::OperatorHandle _mass_rhs;

private:
  /// Downcast to the complex eigenproblem equation system held by the base operator.
  ComplexEigenproblemEquationSystem * GetComplexEigenproblemEquationSystem() const
  {
    return static_cast<ComplexEigenproblemEquationSystem *>(GetEquationSystem());
  }

  /// Assemble the eigenproblem operator and mass matrix.
  void BuildEquationSystemOperator();

  /// Distribute the computed eigenvectors onto the complex grid functions.
  void RecoverEigenproblemSolution(EigensolverBase * eigensolver);
};

} // namespace Moose::MFEM

#endif
