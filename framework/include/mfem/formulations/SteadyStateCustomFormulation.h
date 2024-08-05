#pragma once

#include "MFEMFormulation.h"
#include "steady_state_equation_system_problem_builder.h"

/**
 * SteadyStateCustomFormulation
 *
 * SteadyStateCustomFormulation is an MFEM formulation to be used for time invariant problems
 * with a Steady executioner.
 */
class SteadyStateCustomFormulation : public MFEMFormulation
{
public:
  static InputParameters validParams();

  SteadyStateCustomFormulation(const InputParameters & parameters);
  ~SteadyStateCustomFormulation() override = default;

  /**
   * Return a shared pointer to the Platypus ProblemBuilder
   */
  std::shared_ptr<platypus::ProblemBuilder> getProblemBuilder() const override
  {
    return _formulation;
  }

private:
  // Shared pointer to the ProblemBuilder
  const std::shared_ptr<platypus::SteadyStateEquationSystemProblemBuilder> _formulation{nullptr};
};
