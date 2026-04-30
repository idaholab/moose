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

#include "EquationSystemProblemOperator.h"
#include "EigenproblemEquationSystem.h"

namespace Moose::MFEM
{
/// Steady-state eigenproblem operator with an equation system.
class EigenproblemESProblemOperator : public EquationSystemProblemOperator
{
public:
  EigenproblemESProblemOperator(MFEMProblem & problem) : EquationSystemProblemOperator(problem)
  {
    mooseAssert(std::dynamic_pointer_cast<EigenproblemEquationSystem>(_problem_data.eqn_system),
                "MFEMProblemData::eqn_system is not an EigenproblemEquationSystem.");
  }

  virtual void Solve() override;

  [[nodiscard]] EigenproblemEquationSystem * GetEquationSystem() const override
  {
    return static_cast<EigenproblemEquationSystem *>(
        EquationSystemProblemOperator::GetEquationSystem());
  }

  mfem::OperatorHandle _mass_rhs;

private:
  void RecoverEigenproblemSolution(Moose::MFEM::GridFunctions & gridfunctions,
                                   MFEMEigensolverBase * eigensolver);

protected:
  /// Add kernels/bcs and assemble the linear part of the equation system
  void BuildEquationSystemOperator();
};

} // namespace Moose::MFEM

#endif
