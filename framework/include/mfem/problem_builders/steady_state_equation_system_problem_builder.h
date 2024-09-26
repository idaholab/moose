#pragma once
#include "equation_system_problem_operator.h"
#include "problem_builder_base.h"
#include "steady_state_problem_builder.h"
#include "equation_system_interface.h"

namespace platypus
{

/// Steady-state problems with an equation system.
class SteadyStateEquationSystemProblem : public SteadyStateProblem, public EquationSystemInterface
{
public:
  SteadyStateEquationSystemProblem() = default;
  ~SteadyStateEquationSystemProblem() override = default;

  [[nodiscard]] EquationSystemProblemOperator * GetOperator() const override
  {
    return static_cast<EquationSystemProblemOperator *>(SteadyStateProblem::GetOperator());
  }

  void SetOperator(std::unique_ptr<EquationSystemProblemOperator> problem_operator)
  {
    SteadyStateProblem::SetOperator(std::move(problem_operator));
  }

  void ConstructOperator() override
  {
    auto equation_system = std::make_unique<platypus::EquationSystem>();
    auto problem_operator = std::make_unique<platypus::EquationSystemProblemOperator>(
        *this, std::move(equation_system));

    SetOperator(std::move(problem_operator));
  }

  [[nodiscard]] platypus::EquationSystem * GetEquationSystem() const override
  {
    return GetOperator()->GetEquationSystem();
  }
};

} // namespace platypus
