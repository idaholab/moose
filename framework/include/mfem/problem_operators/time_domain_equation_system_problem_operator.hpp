#pragma once
#include "../common/pfem_extras.hpp"
#include "time_domain_problem_operator.hpp"
#include "problem_operator_interface.hpp"
#include "equation_system_interface.hpp"

namespace hephaestus
{

/// Problem operator for time-dependent problems with an equation system.
class TimeDomainEquationSystemProblemOperator : public TimeDomainProblemOperator,
                                                public EquationSystemInterface
{
public:
  TimeDomainEquationSystemProblemOperator(hephaestus::Problem &) = delete;
  TimeDomainEquationSystemProblemOperator(
      hephaestus::Problem & problem,
      std::unique_ptr<hephaestus::TimeDependentEquationSystem> equation_system)
    : TimeDomainProblemOperator(problem), _equation_system{std::move(equation_system)}
  {
  }

  void SetGridFunctions() override;
  void Init(mfem::Vector & X) override;

  void ImplicitSolve(const double dt, const mfem::Vector & X, mfem::Vector & dX_dt) override;

  [[nodiscard]] hephaestus::TimeDependentEquationSystem * GetEquationSystem() const override
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
  std::unique_ptr<hephaestus::TimeDependentEquationSystem> _equation_system{nullptr};
};

} // namespace hephaestus