#pragma once
#include "problem_operator.h"
#include "problem_operator_interface.h"
#include "equation_system_interface.h"

namespace platypus
{
/// Steady-state problem operator with an equation system.
class EquationSystemProblemOperator : public ProblemOperator, public EquationSystemInterface
{
public:
  EquationSystemProblemOperator(platypus::Problem &) = delete;

  EquationSystemProblemOperator(platypus::Problem & problem,
                                std::unique_ptr<platypus::EquationSystem> equation_system)
    : ProblemOperator(problem), _equation_system(std::move(equation_system))
  {
  }

  void SetGridFunctions() override;
  void Init(mfem::Vector & X) override;

  ~EquationSystemProblemOperator() override = default;

  [[nodiscard]] platypus::EquationSystem * GetEquationSystem() const override
  {
    if (!_equation_system)
    {
      MFEM_ABORT("No equation system has been added to ProblemOperator.");
    }

    return _equation_system.get();
  }

private:
  std::unique_ptr<platypus::EquationSystem> _equation_system{nullptr};
};

} // namespace platypus