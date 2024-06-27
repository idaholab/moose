#pragma once

#include "MFEMFormulation.h"
#include "time_domain_em_formulation.h"

class CustomFormulation : public MFEMFormulation
{
public:
  static InputParameters validParams();

  CustomFormulation(const InputParameters & parameters);
  virtual ~CustomFormulation();

  virtual void execute() override {}
  virtual void initialize() override {}
  virtual void finalize() override {}

  std::shared_ptr<platypus::ProblemBuilder> getProblemBuilder() override { return formulation; }

private:
  std::shared_ptr<platypus::TimeDomainEMFormulation> formulation{nullptr};
};
