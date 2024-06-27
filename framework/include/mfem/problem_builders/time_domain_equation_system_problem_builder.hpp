#pragma once
#include "problem_builder_base.hpp"
#include "time_domain_problem_builder.hpp"
#include "time_domain_equation_system_problem_operator.hpp"
#include "equation_system_interface.hpp"

namespace hephaestus
{
/// Time-depent problems with an equation system.
class TimeDomainEquationSystemProblem : public TimeDomainProblem, public EquationSystemInterface
{
public:
  TimeDomainEquationSystemProblem() = default;
  ~TimeDomainEquationSystemProblem() override = default;

  [[nodiscard]] hephaestus::TimeDomainEquationSystemProblemOperator * GetOperator() const override
  {
    return static_cast<hephaestus::TimeDomainEquationSystemProblemOperator *>(
        TimeDomainProblem::GetOperator());
  }

  void
  SetOperator(std::unique_ptr<hephaestus::TimeDomainEquationSystemProblemOperator> problem_operator)
  {
    TimeDomainProblem::SetOperator(std::move(problem_operator));
  }

  void ConstructOperator() override
  {
    auto equation_system = std::make_unique<hephaestus::TimeDependentEquationSystem>();
    auto problem_operator = std::make_unique<hephaestus::TimeDomainEquationSystemProblemOperator>(
        *this, std::move(equation_system));

    SetOperator(std::move(problem_operator));
  }

  [[nodiscard]] TimeDependentEquationSystem * GetEquationSystem() const override
  {
    return GetOperator()->GetEquationSystem();
  }
};

// Problem-builder for TimeDomainEquationSystemProblem.
class TimeDomainEquationSystemProblemBuilder : public TimeDomainProblemBuilder,
                                               public EquationSystemProblemBuilderInterface
{
public:
  /// NB: set "_problem" member variable in parent class.
  TimeDomainEquationSystemProblemBuilder()
    : TimeDomainProblemBuilder(new hephaestus::TimeDomainEquationSystemProblem)
  {
  }

  ~TimeDomainEquationSystemProblemBuilder() override = default;

  /// NB: - note use of final. Ensure that the equation system is initialized.
  void InitializeKernels() final;

  auto ReturnProblem() { return ProblemBuilder::ReturnProblem<TimeDomainEquationSystemProblem>(); }

protected:
  [[nodiscard]] hephaestus::TimeDomainEquationSystemProblem * GetProblem() const override
  {
    return ProblemBuilder::GetProblem<hephaestus::TimeDomainEquationSystemProblem>();
  }

  [[nodiscard]] hephaestus::TimeDependentEquationSystem * GetEquationSystem() const override
  {
    return GetProblem()->GetEquationSystem();
  }
};

} // namespace hephaestus
