#pragma once
#include "problem_builder_base.h"
#include "time_domain_problem_operator.h"

namespace platypus
{

/// Time-dependent problems with no equation system.
class TimeDomainProblem : public Problem
{
public:
  [[nodiscard]] platypus::TimeDomainProblemOperator * GetOperator() const override
  {
    if (!_problem_operator)
    {
      MFEM_ABORT("No operator has been added.");
    }

    return _problem_operator.get();
  }

  void SetOperator(std::unique_ptr<platypus::TimeDomainProblemOperator> problem_operator)
  {
    _problem_operator.reset();
    _problem_operator = std::move(problem_operator);
  }

  void ConstructOperator() override
  {
    _problem_operator.reset();
    _problem_operator = std::make_unique<platypus::TimeDomainProblemOperator>(*this);
  }

private:
  std::unique_ptr<platypus::TimeDomainProblemOperator> _problem_operator{nullptr};
};

/// Problem-builder for TimeDomainProblem.
class TimeDomainProblemBuilder : public ProblemBuilder
{
public:
  TimeDomainProblemBuilder() : ProblemBuilder(new platypus::TimeDomainProblem) {}

  ~TimeDomainProblemBuilder() override = default;

  static std::vector<mfem::ParGridFunction *>
  RegisterTimeDerivatives(std::vector<std::string> gridfunction_names,
                          platypus::GridFunctions & gridfunctions);

  void RegisterGridFunctions() override;

  void SetOperatorGridFunctions() override;

  void ConstructOperator() override;

protected:
  /// NB: constructor called in derived classes.
  TimeDomainProblemBuilder(platypus::TimeDomainProblem * problem) : ProblemBuilder(problem) {}

  [[nodiscard]] platypus::TimeDomainProblem * GetProblem() const override
  {
    return ProblemBuilder::GetProblem<platypus::TimeDomainProblem>();
  };
};

} // namespace platypus
