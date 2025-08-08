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

namespace Moose::MFEM
{
/// Steady-state problem operator with an equation system.
class ComplexEquationSystemProblemOperator : public EquationSystemProblemOperator
{

public:
  ComplexEquationSystemProblemOperator(MFEMProblemData & problem)
    : EquationSystemProblemOperator(problem),
      _equation_system{
          std::dynamic_pointer_cast<Moose::MFEM::ComplexEquationSystem>(problem.eqn_system)}
  {
  }

  void SetGridFunctions() override;
  void Init(mfem::BlockVector & X) override;
  virtual void Solve(mfem::Vector & X) override;

  [[nodiscard]] Moose::MFEM::ComplexEquationSystem * GetEquationSystem() const override
  {
    if (!_equation_system)
    {
      MFEM_ABORT("No equation system has been added to ProblemOperator.");
    }

    return _equation_system.get();
  }

private:
  std::shared_ptr<Moose::MFEM::ComplexEquationSystem> _equation_system{nullptr};
  std::vector<mfem::ParComplexGridFunction *> _cpx_trial_variables;
};

} // namespace Moose::MFEM

#endif
