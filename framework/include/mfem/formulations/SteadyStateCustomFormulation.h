#pragma once

#include "MFEMFormulation.h"
#include "steady_state_equation_system_problem_builder.h"

class SteadyStateCustomFormulation : public MFEMFormulation
{
public:
  static InputParameters validParams();

  SteadyStateCustomFormulation(const InputParameters & parameters);
  ~SteadyStateCustomFormulation() override = default;

  std::shared_ptr<platypus::ProblemBuilder> getProblemBuilder() const override
  {
    return _formulation;
  }

private:
  const std::shared_ptr<platypus::SteadyStateEquationSystemProblemBuilder> _formulation{nullptr};
};
