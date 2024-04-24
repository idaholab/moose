#pragma once

#include "MFEMFormulation.h"
#include "factory.hpp"

class CustomFormulation : public MFEMFormulation
{
public:
  static InputParameters validParams();

  CustomFormulation(const InputParameters & parameters);
  virtual ~CustomFormulation();

  virtual void execute() override {}
  virtual void initialize() override {}
  virtual void finalize() override {}

  std::shared_ptr<hephaestus::ProblemBuilder> getProblemBuilder() override { return formulation; }

private:
  std::shared_ptr<hephaestus::TimeDomainEMFormulation> formulation{nullptr};
};
