#pragma once

#include "MFEMFormulation.h"
#include "time_domain_equation_system_problem_builder.h"

class CustomFormulation : public MFEMFormulation
{
public:
  static InputParameters validParams();

  CustomFormulation(const InputParameters & parameters);
  ~CustomFormulation() override = default;

  virtual void execute() override {}
  virtual void initialize() override {}
  virtual void finalize() override {}

  /// Returns a shared pointer to the time-domain equation system problem builder.
  std::shared_ptr<platypus::ProblemBuilder> getProblemBuilder() const override { return _formulation; }

private:
  const std::shared_ptr<platypus::TimeDomainEquationSystemProblemBuilder> _formulation{nullptr};
};
