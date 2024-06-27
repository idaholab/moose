#pragma once
#include "problem_operator.h"
#include "problem_builder_base.h"
namespace platypus
{

/// Class for steady-state problems with no equation system.
class SteadyStateProblem : public Problem
{
public:
  SteadyStateProblem() = default;
  ~SteadyStateProblem() override = default;

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

class SteadyStateProblemBuilder : public ProblemBuilder
{
public:
  SteadyStateProblemBuilder() : ProblemBuilder(new platypus::SteadyStateProblem) {}

  ~SteadyStateProblemBuilder() override = default;

  auto ReturnProblem() { return ProblemBuilder::ReturnProblem<SteadyStateProblem>(); }

  void RegisterFESpaces() override {}

  void RegisterGridFunctions() override {}

  void RegisterAuxSolvers() override {}

  void RegisterCoefficients() override {}

  void SetOperatorGridFunctions() override;

  void ConstructOperator() override;

  void ConstructState() override;

  void ConstructTimestepper() override {}

protected:
  // NB: constructor for derived classes.
  SteadyStateProblemBuilder(platypus::SteadyStateProblem * problem) : ProblemBuilder(problem) {}

  [[nodiscard]] platypus::SteadyStateProblem * GetProblem() const override
  {
    return ProblemBuilder::GetProblem<platypus::SteadyStateProblem>();
  }
};

} // namespace platypus
