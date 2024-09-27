#pragma once
#include "problem_operator.h"
#include "problem_builder_base.h"
namespace platypus
{

/// Class for steady-state problems with no equation system.
class SteadyStateProblemData : public ProblemData
{
public:
  SteadyStateProblemData() = default;
  ~SteadyStateProblemData() override = default;

  [[nodiscard]] platypus::ProblemOperator * GetOperator() const override
  {
    if (!_problem_operator)
    {
      MFEM_ABORT("No operator has been added.");
    }

    return _problem_operator.get();
  }

  void SetOperator(std::unique_ptr<platypus::ProblemOperator> problem_operator)
  {
    _problem_operator.reset();
    _problem_operator = std::move(problem_operator);
  }

  void ConstructOperator() override
  {
    _problem_operator.reset();
    _problem_operator = std::make_unique<platypus::ProblemOperator>(*this);
  }

private:
  std::unique_ptr<platypus::ProblemOperator> _problem_operator{nullptr};
};

} // namespace platypus
