#pragma once

#include "MFEMFormulation.h"
#include "time_domain_equation_system_problem_builder.h"

class TransientCustomFormulation : public MFEMFormulation
{
public:
  static InputParameters validParams();

  TransientCustomFormulation(const InputParameters & parameters);
  ~TransientCustomFormulation() override = default;

  std::shared_ptr<platypus::ProblemBuilder> getProblemBuilder() const override
  {
    return _formulation;
  }

private:
  const std::shared_ptr<platypus::TimeDomainEquationSystemProblemBuilder> _formulation{nullptr};
};
