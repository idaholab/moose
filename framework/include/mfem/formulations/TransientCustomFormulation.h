#pragma once

#include "MFEMFormulation.h"
#include "time_domain_equation_system_problem_builder.h"

/**
 * TransientCustomFormulation
 *
 * TransientCustomFormulation is an MFEM formulation to be used for time dependent problems
 * with a Transient executioner.
 */
class TransientCustomFormulation : public MFEMFormulation
{
public:
  static InputParameters validParams();

  TransientCustomFormulation(const InputParameters & parameters);
  ~TransientCustomFormulation() override = default;

  /**
   * Return a shared pointer to the Platypus ProblemBuilder
   */
  std::shared_ptr<platypus::ProblemBuilder> getProblemBuilder() const override
  {
    return _formulation;
  }

private:
  // Shared pointer to the ProblemBuilder
  const std::shared_ptr<platypus::TimeDomainEquationSystemProblemBuilder> _formulation{nullptr};
};
