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
#include "TimeDomainProblemOperator.h"
#include "EquationSystemInterface.h"

namespace Moose::MFEM
{

/// Problem operator for time-dependent problems with an equation system.
class TimeDomainEquationSystemProblemOperator : public TimeDomainProblemOperator,
                                                public EquationSystemInterface
{
public:
  TimeDomainEquationSystemProblemOperator(MFEMProblem & problem)
    : TimeDomainProblemOperator(problem),
      _equation_system(
          std::dynamic_pointer_cast<TimeDependentEquationSystem>(_problem_data.eqn_system))
  {
  }

  void SetGridFunctions() override;
  void Init(mfem::BlockVector & X) override;
  void ImplicitSolve(const mfem::real_t dt, const mfem::Vector & X, mfem::Vector & dX_dt) override;
  void Solve() override;

  [[nodiscard]] Moose::MFEM::TimeDependentEquationSystem * GetEquationSystem() const override
  {
    if (!_equation_system)
    {
      MFEM_ABORT("No equation system has been added.");
    }

    return _equation_system.get();
  }

protected:
  void BuildEquationSystemOperator(mfem::real_t dt);

private:
  std::shared_ptr<Moose::MFEM::TimeDependentEquationSystem> _equation_system{nullptr};
};

} // namespace Moose::MFEM

#endif
