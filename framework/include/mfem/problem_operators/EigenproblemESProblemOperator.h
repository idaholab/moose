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
class EigensolverBase;

/// Steady-state eigenproblem operator with an equation system.
class EigenproblemESProblemOperator : public EquationSystemProblemOperator
{
public:
  EigenproblemESProblemOperator(MFEMProblem & problem, const std::string & weak_form_name = "");

  virtual void Solve() override;

  [[nodiscard]] EigenproblemEquationSystem * GetEquationSystem() const override
  {
    return cast_ptr<EigenproblemEquationSystem *>(
        EquationSystemProblemOperator::GetEquationSystem());
  }

private:
  void RecoverEigenproblemSolution(GridFunctions & gridfunctions, EigensolverBase * eigensolver);

protected:
  /// Form the equation-system operator used by the eigensolver.
  void FormEquationSystemOperator();
};

} // namespace Moose::MFEM

#endif
