#pragma once
#include "problem_builder_base.h"
#include "time_domain_problem_operator.h"

namespace platypus
{

/// Time-dependent problems with no equation system.
class TimeDomainProblemData : public ProblemData
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

} // namespace platypus
