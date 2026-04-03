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

#include "TimeDependentProblemOperator.h"
#include "EquationSystemInterface.h"
#include "TimeDependentEquationSystem.h"

namespace Moose::MFEM
{

/// Problem operator for time-dependent problems with an equation system.
class TimeDependentEquationSystemProblemOperator : public TimeDependentProblemOperator,
                                                   public EquationSystemInterface
{
public:
  TimeDependentEquationSystemProblemOperator(MFEMProblem & problem)
    : TimeDependentProblemOperator(problem),
      _equation_system(
          std::dynamic_pointer_cast<TimeDependentEquationSystem>(_problem_data.eqn_system))
  {
  }

  virtual void SetGridFunctions() override;
  virtual void Init(mfem::BlockVector & X) override;
  virtual void ImplicitSolve(const mfem::real_t, const mfem::Vector &, mfem::Vector &) override;
  virtual void Solve() override;

  [[nodiscard]] virtual Moose::MFEM::TimeDependentEquationSystem *
  GetEquationSystem() const override
  {
    mooseAssert(_equation_system,
                "No TimeDependentEquationSystem in TimeDependentEquationSystemProblemOperator.");
    return _equation_system.get();
  }

protected:
  void BuildEquationSystemOperator(mfem::real_t dt);

private:
  std::shared_ptr<Moose::MFEM::TimeDependentEquationSystem> _equation_system{nullptr};
};

} // namespace Moose::MFEM

#endif
