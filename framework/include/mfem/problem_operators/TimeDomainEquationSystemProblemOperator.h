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
  TimeDomainEquationSystemProblemOperator(MFEMProblemData & problem)
    : TimeDomainProblemOperator(problem),
      _equation_system(
          std::dynamic_pointer_cast<Moose::MFEM::TimeDependentEquationSystem>(problem.eqn_system))
  {
  }

  void SetGridFunctions() override;
  void Init(mfem::BlockVector & X) override;

  void ImplicitSolve(const double dt, const mfem::Vector & X, mfem::Vector & dX_dt) override;

  [[nodiscard]] Moose::MFEM::TimeDependentEquationSystem * GetEquationSystem() const override
  {
    if (!_equation_system)
    {
      MFEM_ABORT("No equation system has been added.");
    }

    return _equation_system.get();
  }

protected:
  void BuildEquationSystemOperator(double dt);

private:
  std::vector<mfem::ParGridFunction *> _trial_variable_time_derivatives;
  std::shared_ptr<Moose::MFEM::TimeDependentEquationSystem> _equation_system{nullptr};
};

} // namespace Moose::MFEM

#endif
