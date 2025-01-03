#pragma once
#include "mfem/miniapps/common/pfem_extras.hpp"
#include "time_domain_problem_operator.h"
#include "problem_operator_interface.h"
#include "equation_system_interface.h"

namespace platypus
{

/// Problem operator for time-dependent problems with an equation system.
class TimeDomainEquationSystemProblemOperator : public TimeDomainProblemOperator,
                                                public EquationSystemInterface
{
public:
  TimeDomainEquationSystemProblemOperator(MFEMProblemData & problem)
    : TimeDomainProblemOperator(problem),
      _equation_system(
          std::dynamic_pointer_cast<platypus::TimeDependentEquationSystem>(problem._eqn_system))
  {
  }

  void SetGridFunctions() override;
  void Init(mfem::BlockVector & X) override;

  void ImplicitSolve(const double dt, const mfem::Vector & X, mfem::Vector & dX_dt) override;

  [[nodiscard]] platypus::TimeDependentEquationSystem * GetEquationSystem() const override
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
  std::shared_ptr<platypus::TimeDependentEquationSystem> _equation_system{nullptr};
};

} // namespace platypus
